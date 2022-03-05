import struct
import sys

class FlowFrame():
    def __init__(self, flowFile, indx):
        self.index = -1
        try:
            inputFile = open(flowFile, "rb")
        except IOError:
            inputFile.close()
            print("No .flo files in Flo Directory")
            return
        
        self.index = indx
        print(f"Reading flow vectors from: {flowFile}")
        dummy = struct.unpack('f', inputFile.read(4))[0]
        self.width = struct.unpack('i', inputFile.read(4))[0]
        self.height = struct.unpack('i', inputFile.read(4))[0]
        self.xFlow = ""
        self.yFlow = ""
        for i in range(self.width * self.height):
           x = struct.unpack('f', inputFile.read(4))[0]
           y = struct.unpack('f', inputFile.read(4))[0]

           self.xFlow += ',' + str(x)
           self.yFlow += ',' + str(y)
        
        inputFile.close()
    
    def getFlowFrameIndex(self):
        return self.index

    def getWidth(self):
        return self.width

    def getHeight(self):
        return self.height

    def getXFlow(self):
        return self.xFlow

    def getYFlow(self):
        return self.yFlow
    
    
        
        
    