import os
import fnmatch
from PIL import Image
from torchvision import transforms
from config import *


class FrameReader():
    def __init__(self, videoPath, slowdown):
        self.videoPath = videoPath
        self.slowdown = slowdown
        self.frameCount = 0
        self.frameWidth = 0
        self.frameHeight = 0

        self.extractVideoFrames()

    # 
    # Function extracts frames from video into output directory using ffmpeg
    #
    def extractVideoFrames(self):
        retval = os.system(f'ffmpeg -i {self.videoPath} -vsync 0 {dirPaths["rFrames"]}/_{"%05d"}.png')
        if retval:
            print ("Error converting video file.")
            return
        
        # extract frame count
        self.frameCount = len(fnmatch.filter(os.listdir(dirPaths["rFrames"]), '*.png'))

        # set frame width and height by opening first file
        with open(dirPaths["rFrames"] + '/' + FIRST_RAW_FRAME_FILENAME, 'rb') as file:
            firstFrame = Image.open(file).convert('RGB')
            self.frameWidth, self.frameHeight = firstFrame.size

        # rename frame files to give space for intermediate frames
        index = 0
        for _, filename in enumerate(sorted(os.listdir(dirPaths["rFrames"]))):
            if filename.endswith('.png'):
                os.rename(dirPaths["rFrames"] + '/' + filename, dirPaths["iFrames"] + '/_' + str(index).zfill(5) + '.png')
                index += self.slowdown

    def getFrameCount(self):
        return self.frameCount

    def getWidth(self):
        return self.frameWidth

    def getHeight(self):
        return self.frameHeight  

    def getFramesByFirstIndex(self, firstFrameIndex):
        I0, I1 = None, None

        with open(dirPaths["iFrames"] + '/_%05d.png' % firstFrameIndex, 'rb') as file:
            I0 = Image.open(file).convert('RGB')
        with open(dirPaths["iFrames"] + '/_%05d.png' % (firstFrameIndex + self.slowdown), 'rb') as file:
            I1 = Image.open(file).convert('RGB')

        I0 = transforms.ToTensor()(I0).unsqueeze_(0)
        I1 = transforms.ToTensor()(I1).unsqueeze_(0)

        return I0, I1
