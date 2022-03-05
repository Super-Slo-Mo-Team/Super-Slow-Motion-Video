import sys
import zmq
import struct
from FlowFrame import FlowFrame

RESPONSE_SUCCESS = b'0'
RESPONSE_FAILURE = b'1'

def main():
    if len(sys.argv) != 2:
        print("Incorrect invocation. Exiting.")
        exit()

    port = "8080"
    context = zmq.Context()
    flowRequester = context.socket(zmq.REQ)
    flowRequester.connect("tcp://localhost:%s" % port)

    flowPath = sys.argv[1]
    flowFrameIndex = 0

    while True:

        file = (f"{flowPath}\\_{flowFrameIndex:05}.flo")
        
        frame = FlowFrame(file, flowFrameIndex)

        if (frame.getFlowFrameIndex == -1):
            print("Finished reading flow vectors")
            break

        msg = (f"{frame.getFlowFrameIndex()},{frame.getWidth()},{frame.getHeight()}:{frame.getXFlow()}:{frame.getXFlow()}")
        
        flowRequester.send(msg.encode())

        response = flowRequester.recv()
        if (response == RESPONSE_SUCCESS):
            flowFrameIndex += 1
        else:
            print(f"Frame: {flowFrameIndex} failed to send. Trying again...")



if __name__ == "__main__":
    main()