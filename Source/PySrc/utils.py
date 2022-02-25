import os
from os.path import exists
import fnmatch
from pathlib import Path
from PIL import Image
from torchvision import transforms
from config import *
import shutil


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
        if not exists(Path(f'{dirPaths["FRAME"]}/f_00001.png')):
            self.populate()
        self.format()

    

    ##TODO need to integrate calling NVIDIA API here after ffmpeg call
    def populate(self):
        path = Path(f'{dirPaths["FRAME"]}/f_%05d.png')
        retval = os.system(f'ffmpeg -i {self.videoPath} -vsync 0 {path}')#### added f to these frames
        if retval:
            print ("Error converting video file.")
            return
        
        ### DO FLO GENERATION HERE, output = {dirPaths['FLO']}
        # extract frame count
    
    
    def format(self):
        self.frameCount = len(fnmatch.filter(os.listdir(f'{dirPaths["FRAME"]}'), '*.png'))# does this need to be changed for windows?

        # set frame width and height by opening first file
        path = Path(f'{dirPaths["FRAME"]}/{FIRST_RAW_FRAME_FILENAME}')
        with open(path, 'rb') as file:
            firstFrame = Image.open(file).convert('RGB')
            self.frameWidth, self.frameHeight = firstFrame.size
            
        shutil.copytree(dirPaths['FRAME'], dirPaths['TMP'])
        index = 0
        for _, filename in enumerate(sorted(os.listdir(dirPaths['TMP']))):
            if filename.endswith('.png'):
                src = Path(f"{dirPaths['TMP']}/{filename}")
                dst = Path(f"{dirPaths['TMP']}/_{str(index).zfill(5)}.png")
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
        path = Path(f'{dirPaths["TMP"]}/_')
        with open(f'{path}%05d.png' % firstFrameIndex, 'rb') as file:
            I0 = Image.open(file).convert('RGB')
        with open(f'{path}%05d.png' % (firstFrameIndex + self.slowdown), 'rb') as file:
            I1 = Image.open(file).convert('RGB')

        I0 = transforms.ToTensor()(I0).unsqueeze_(0)
        I1 = transforms.ToTensor()(I1).unsqueeze_(0)

        return I0, I1


def removeDir(path):
    shutil.rmtree(path)

