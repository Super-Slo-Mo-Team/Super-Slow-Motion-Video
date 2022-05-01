import argparse
import os
import threading
import zmq
import numpy as np
import torch
from torchvision import transforms
from utils import FrameReader
from utils import removeDir
from config import *
import model as model
from pathlib import Path
#
# Program entry point
#
def generateSlowMotion(fvgEvent):
    # Parser for command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', type = str, required = True, help='path of video to be converted')
    parser.add_argument('--output', type = str, required = True, help='path of converted video')
    parser.add_argument('--slowdown', type = int, required = True, help='slows down video by specified factor')
    parser.add_argument('--FPS', type = int, required = True, help='FPS of output video')
    args = parser.parse_args()
    
    if os.path.exists(dirPaths["TMP"]):
        print("Removing TmpDir...")
        removeDir(Path(f'./{dirPaths["TMP"]}'))
    
    if not os.path.exists(dirPaths["OUT"]):
        os.makedirs(dirPaths["OUT"])
    

    # TODO: create frame reader and generate frames on init
    frr = FrameReader(args.input, args.slowdown)
    # TODO: create models here

    
    ArbTimeFlowIntrp = model.UNet(20, 5)
    ArbTimeFlowIntrp.to('cpu')
    for param in ArbTimeFlowIntrp.parameters():
        param.requires_grad = False

    flowBackWarp = model.backWarp(frr.getWidth(), frr.getHeight(), 'cpu')
    flowBackWarp = flowBackWarp.to('cpu')

    dict1 = torch.load(Path(f'{ROOT}/SuperSloMo.ckpt'), map_location='cpu')
    ArbTimeFlowIntrp.load_state_dict(dict1['state_dictAT'])

    # create socket connection
    context = zmq.Context()
    flowReceiver = context.socket(zmq.REP)
    print ('Connecting receiver to localhost server on port 8080...')
    flowReceiver.bind('tcp://*:8080')

    firstFrameIndex = 0
    lastFrameIndex = frr.getFrameCount() * args.slowdown
    # set event to start flow vector generation thread
    fvgEvent.set()

    # process two frames at a time
    while firstFrameIndex < lastFrameIndex - args.slowdown:
        # wait for client request
        
        msg = flowReceiver.recv().decode('utf-8')
        # retrieve message metadata
        msgSplit = msg.split(':')
        
        
        metadata = [int(x) for x in msgSplit[0].split(',')]
        vectorIndex, vectorWidth, vectorHeight = metadata[0], metadata[1], metadata[2]
        
        # send failure reply back to client
        if firstFrameIndex != vectorIndex * args.slowdown:
            print ('SlowMotion.py: Received flow vectors out of order. Retrying.')
            flowReceiver.send_string(str(RESPONSE_FAILURE))
            continue
        else:
            print (f'SlowMotion.py: Received flow vector for idx: {vectorIndex}')

        # retrieve xFlow and yFlow
        xFlow = msgSplit[1].split(',')[1:]
        yFlow = msgSplit[2].split(',')[1:]

        # create flow tensors
        xFlow = torch.tensor(np.array(xFlow, dtype = np.float32))
        yFlow = torch.tensor(np.array(yFlow, dtype = np.float32))
        xFlow = torch.reshape(xFlow, (1, 1, vectorHeight, vectorWidth))
        yFlow = torch.reshape(yFlow, (1, 1, vectorHeight, vectorWidth))
        F_0_1 = torch.cat((xFlow, yFlow), dim = 1)
        F_1_0 = np.negative(F_0_1)

        # retrieve frames and load them into tensors
        I0, I1 = frr.getFramesByFirstIndex(firstFrameIndex)
        I0, I1 = I0.to('cpu'), I1.to('cpu')

        # Generate intermediate frames
        for i in range(1, args.slowdown):
            # TODO: Consider whether we need transforms on the frames
            # TODO: perform calculations
            t = float(i) / args.slowdown
            temp = -t * (1 - t)
            fCoeff = [temp, t * t, (1 - t) * (1 - t), temp]

            F_t_0 = fCoeff[0] * F_0_1 + fCoeff[1] * F_1_0
            F_t_1 = fCoeff[2] * F_0_1 + fCoeff[3] * F_1_0

            g_I0_F_t_0 = flowBackWarp(I0, F_t_0)
            g_I1_F_t_1 = flowBackWarp(I1, F_t_1)

            intrpOut = ArbTimeFlowIntrp(torch.cat((I0, I1, F_0_1, F_1_0, F_t_1, F_t_0, g_I1_F_t_1, g_I0_F_t_0), dim=1))

            F_t_0_f = intrpOut[:, :2, :, :] + F_t_0
            F_t_1_f = intrpOut[:, 2:4, :, :] + F_t_1
            V_t_0   = torch.sigmoid(intrpOut[:, 4:5, :, :])
            V_t_1   = 1 - V_t_0

            g_I0_F_t_0_f = flowBackWarp(I0, F_t_0_f)
            g_I1_F_t_1_f = flowBackWarp(I1, F_t_1_f)

            wCoeff = [1 - t, t]

            Ft_p = (wCoeff[0] * V_t_0 * g_I0_F_t_0_f + wCoeff[1] * V_t_1 * g_I1_F_t_1_f) / (wCoeff[0] * V_t_0 + wCoeff[1] * V_t_1)

            # Save intermediate frame
            interpolatedFrame = transforms.ToPILImage()(Ft_p[0].squeeze_(0))
            path = Path(f'{dirPaths["TMP"]}/_')
            interpolatedFrame.save(f'{path}%05d.png' % (firstFrameIndex + i))

        # send success reply back to client
        flowReceiver.send_string(str(RESPONSE_SUCCESS))

        # increment firstFrameIndex
        firstFrameIndex += args.slowdown

    # reconstruct video at specified FPS
    path = Path(f'{dirPaths["TMP"]}/_')
    retval = os.system(f'ffmpeg -r {args.FPS} -i {path}%05d.png -vcodec ffvhuff {args.output}')

    #clean up temp directory
    path = Path(f'./{dirPaths["TMP"]}')
    removeDir(path)
    if retval:
        print ('Error creating output video.')

    # TODO: remove tmp directories

def generateFlowVectors(fvgEvent):
    if fvgEvent.wait(10):
        print("Starting flow vector generation...")
        target = Path(f'{ROOT}/Source/PySrc/flowvectors.py') 
        os.system(f'python {target} {dirPaths["FLO"]}')
    else:
        print("Frame generation timed out. Exiting")
        exit(1)

def main():
    # define flow vector generation trigger
    fvgEvent = threading.Event()

    # create two threads to trigger flow generation script and slow motion generation script
    slowmotionThread = threading.Thread(target = generateSlowMotion, args = [fvgEvent])
    flowVectorsThread = threading.Thread(target = generateFlowVectors, args = [fvgEvent])

    # terminate threads when slow motion generation finishes
    slowmotionThread.daemon = True

    # start threads
    slowmotionThread.start()
    flowVectorsThread.start()

    # finish
    slowmotionThread.join()
    print ('Finished creating output video.')

if __name__ == "__main__":
    main()
