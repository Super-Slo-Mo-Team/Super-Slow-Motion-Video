import sys
import zmq
import torch
import numpy as np
from utils import FrameReader

RESPONSE_SUCCESS = 'SUCCESS'
RESPONSE_FAILURE = 'FAILURE'

def main():
    videoPath = sys.argv[1]
    framePath = sys.argv[2]

    # TODO: will be replaced by Sean's work
    # create frame reader (generates frames in output directory on init)
    frr = FrameReader(videoPath, framePath)

    # create socket connection
    context = zmq.Context()
    flowReceiver = context.socket(zmq.REP)
    print ('Connecting receiver to localhost server on port 8080...')
    flowReceiver.bind("tcp://*:8080")

    firstFrameIndex = 0
    lastFrameIndex = frr.getFrameCount()

    # process two frames at a time
    while firstFrameIndex < lastFrameIndex - 1:
        # wait for client request
        msg = flowReceiver.recv().decode('utf-8')
        
        # retrieve message metadata
        msgSplit = msg.split(':')
        metadata = [int(x) for x in msgSplit[0].split(',')]
        vectorIndex, vectorWidth, vectorHeight = metadata[0], metadata[1], metadata[2]

        # send failure reply back to client
        if firstFrameIndex != vectorIndex:
            print ('RECEIVED FLOW VECTORS OUT OF ORDER')
            flowReceiver.send_string(RESPONSE_FAILURE)
            continue

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

        # retrieve frames
        I0, I1 = frr.getFramesByFirstIndex(firstFrameIndex)
        firstFrameIndex += 1

        # TODO: continue computation
        print (F_0_1)
        print (F_1_0)
        print (I0)
        print (I1)

        # send success reply back to client
        flowReceiver.send_string(RESPONSE_SUCCESS)

if __name__ == "__main__":
    main()
