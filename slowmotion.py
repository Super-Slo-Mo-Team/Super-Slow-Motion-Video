import argparse
import os
import threading
import zmq
import numpy as np
import torch
from utils import FrameReader
from config import *

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

    # TODO: integrate with full pipeline
    try:
        os.mkdir(dirPaths['root'])
        os.mkdir(dirPaths['flo'])
        os.mkdir(dirPaths['rFrames'])
        os.mkdir(dirPaths['iFrames'])
    except:
        print ('Remove the tmp directory to proceed...')

    # TODO: create frame reader and generate frames on init
    frr = FrameReader(args.input, args.slowdown)

    # TODO: create models here

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
            print ('PY: Received flow vectors out of order. Retrying.')
            flowReceiver.send_string(str(RESPONSE_FAILURE))
            continue
        else:
            print (f'PY: Received flow vector for idx: {vectorIndex}')

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
            pass

        # send success reply back to client
        flowReceiver.send_string(str(RESPONSE_SUCCESS))

        # increment firstFrameIndex
        firstFrameIndex += args.slowdown

    # reconstruct video at specified FPS
    # retval = os.system(f'ffmpeg -r {args.FPS} -i {dirPaths["iFrames"]}/_%05d.png -vcodec ffvhuff {args.output}')
    # if retval:
    #     print ('Error creating output video.')

    # TODO: remove tmp directories

def generateFlowVectors(fvgEvent):
    if fvgEvent.wait(10):
        print("Starting flow vector generation...")
        os.system(f'./FlowVectors {dirPaths["flo"]}')
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
