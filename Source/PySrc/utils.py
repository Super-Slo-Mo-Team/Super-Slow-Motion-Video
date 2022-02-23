import os
from os.path import exists
import fnmatch
from PIL import Image
from torchvision import transforms
from config import *

####Changed all dirPaths to new ones

##TODO: SPLIT EXTRACT VIDEO FRAMES INTO:
##      POPULATE(): if FrameDir/FloDir are empty we call this to run ffmpeg and NVIDIA Executable
##      FORMAT() : can run this if FrameDir/FloDir are full and it formats all the frames to leave space for interpolation
class FrameReader():
    def __init__(self, videoPath, slowdown):
        self.videoPath = videoPath
        self.slowdown = slowdown
        self.frameCount = 0
        self.frameWidth = 0
        self.frameHeight = 0
        if not exists(f'{dirPaths["FRAME"]}/f_00001.png'):
            self.populate()
        self.format()

    

    ##TODO need to integrate calling NVIDIA API here after ffmpeg call
    def populate(self):
        print("In populate")
        retval = os.system(f'ffmpeg -i {self.videoPath} -vsync 0 {dirPaths["FRAME"]}/f_{"%05d"}.png')#### added f to these frames
        if retval:
            print ("Error converting video file.")
            return
        
        ### DO FLO GENERATION HERE, output = {dirPaths['FLO']}
        # extract frame count
        self.frameCount = len(fnmatch.filter(os.listdir(f'{dirPaths["FRAME"]}'), '*.png'))

        # set frame width and height by opening first file
        with open(dirPaths["FRAME"] + '/' + FIRST_RAW_FRAME_FILENAME, 'rb') as file:
            firstFrame = Image.open(file).convert('RGB')
            self.frameWidth, self.frameHeight = firstFrame.size
    
    
    def format(self):
        os.system(f"cp -r {dirPaths['FRAME']}/. {dirPaths['TMP']} ")
        index = 0
        for _, filename in enumerate(sorted(os.listdir(dirPaths['TMP']))):
            if filename.endswith('.png'):
                src = f"{dirPaths['TMP']}/{filename}"
                dst = f"{dirPaths['TMP']}/_{str(index).zfill(5)}.png"
                os.rename(src, dst)
                index += self.slowdown



    ##No longer in use for now.
    def extractVideoFrames(self):
        retval = os.system(f'ffmpeg -i {self.videoPath} -vsync 0 {dirPaths["FRAME"]}/f_{"%05d"}.png')#### added f to these frames
        if retval:
            print ("Error converting video file.")
            return
        
        # extract frame count
        self.frameCount = len(fnmatch.filter(os.listdir(f'{dirPaths["FRAME"]}'), '*.png'))

        # set frame width and height by opening first file
        with open(dirPaths["FRAME"] + '/' + FIRST_RAW_FRAME_FILENAME, 'rb') as file:
            firstFrame = Image.open(file).convert('RGB')
            self.frameWidth, self.frameHeight = firstFrame.size


        os.system(f"cp -r {dirPaths['FRAME']}/. {dirPaths['TMP']} ")
        # rename frame files to give space for intermediate frames
        index = 0
        for _, filename in enumerate(sorted(os.listdir(dirPaths['TMP']))):
            if filename.endswith('.png'):
                src = f"{dirPaths['TMP']}/{filename}"
                dst = f"{dirPaths['TMP']}/_{str(index).zfill(5)}.png"
                os.rename(src, dst)
                index += self.slowdown

    def getFrameCount(self):
        return self.frameCount

    def getWidth(self):
        return self.frameWidth

    def getHeight(self):
        return self.frameHeight  

    def getFramesByFirstIndex(self, firstFrameIndex):
        I0, I1 = None, None

        with open(dirPaths["TMP"] + '/_%05d.png' % firstFrameIndex, 'rb') as file:
            I0 = Image.open(file).convert('RGB')
        with open(dirPaths["TMP"] + '/_%05d.png' % (firstFrameIndex + self.slowdown), 'rb') as file:
            I1 = Image.open(file).convert('RGB')

        I0 = transforms.ToTensor()(I0).unsqueeze_(0)
        I1 = transforms.ToTensor()(I1).unsqueeze_(0)

        return I0, I1
