import os
from os.path import exists
import fnmatch
from pathlib import Path
from PIL import Image
from torchvision import transforms
from config import *
import shutil
import subprocess


####Changed all dirPaths to new ones


class FrameReader():
    def __init__(self, videoPath, slowdown):
        self.videoPath = videoPath
        self.slowdown = slowdown
        self.frameCount = 0
        self.frameWidth = 0
        self.frameHeight = 0
        if not os.path.exists(dirPaths["FRAME"]):
            self.populate()
        self.format()

    

    ##TODO need to integrate calling NVIDIA API here after ffmpeg call
    def populate(self):
        os.makedirs(dirPaths["FRAME"])
        path = Path(f'{dirPaths["FRAME"]}/f_%05d.png')
        retval = os.system(f'ffmpeg -i {self.videoPath} -vsync 0 {path}')
        if retval:
            print ("Error converting video file.")
            return
        
        if not os.path.exists(dirPaths["FLO"]):
            os.makedirs(dirPaths["FLO"])
            ##TODO##
            ##### generate flow frame and fill dirPaths["FLO"] with the output ####
            print("Starting flow vector generation...")
            subprocess.run(['..\\..\\Source\\AppOFCuda\\AppOFCuda.exe',f'--input={dirPaths["FRAME"]}\\*.png',f'--output={dirPaths["FLO"]}\\'])
    
    def format(self):
        self.frameCount = len(fnmatch.filter(os.listdir(f'{dirPaths["FRAME"]}'), '*.png'))

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

