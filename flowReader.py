import torch
import numpy as np

class FlowReader():
    def __init__(self):
        self.size = 0
        self.width = 0
        self.height = 0
        self.frameFlows = []

    def readFrames(self, path):
        file = open(path, 'r')
        data = file.readline().split(' ')

        self.size = int(data[0])
        self.width = int(data[1])
        self.height = int(data[2])

        for _ in range(self.size):
            xFlow = torch.tensor(np.array(file.readline().split(' ')[:-1], dtype = np.float32))
            yFlow = torch.tensor(np.array(file.readline().split(' ')[:-1], dtype = np.float32))
            xFlow = torch.reshape(xFlow, (1, 1, self.height, self.width))
            yFlow = torch.reshape(yFlow, (1, 1, self.height, self.width))
            frameFlow = torch.cat((xFlow, yFlow), dim = 1)
            self.frameFlows.append(frameFlow)

    def getSize(self):
        return self.size

    def getWidth(self):
        return self.width

    def getHeight(self):
        return self.height

    def getFrameFlows(self):
        return self.frameFlows

    def getSingleFrameFlow(self, index):
        return self.frameFlows[index]
