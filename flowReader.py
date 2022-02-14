import torch
import numpy as np
import sys
class flowReader():
    def __init__(self, path) -> None:
        file = open(path, 'r')
        self.flowFrames = []
        data = file.readline().split(' ')
        self.width = int(data[0])
        self.height = int(data[1])
        numOfFrames = int(data[2]) 
        for frame in range(numOfFrames):
            xFlow = torch.tensor(np.array(file.readline().split(' ')[:-1], dtype = np.float32))
            yFlow = torch.tensor(np.array(file.readline().split(' ')[:-1], dtype = np.float32))
            xFlow = torch.reshape(xFlow, (1,1,self.height,self.width))
            yFlow = torch.reshape(yFlow, (1,1,self.height,self.width))
            currentFrame = torch.cat((xFlow,yFlow), dim = 1)
            self.flowFrames.append(currentFrame)

    def getFlow(self, index):
        return self.flowFrames[index]

    def size(self):
        return len(self.flowFrames)

def main():
    flow = flowReader(sys.argv[1])
    print(flow.getFlow(0))
    print(flow.getFlow(35))
    print(flow.getFlow(0).shape)
    print(flow.size())
    
    

if __name__ == "__main__":
    main()

            


