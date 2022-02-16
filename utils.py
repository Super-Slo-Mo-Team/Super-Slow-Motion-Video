from PIL import Image
import os
import fnmatch


class FrameReader():
    FIRST_FRAME_FILENAME = '_00001.png'

    def __init__(self, videoPath, dirPaths, slowdown):
        self.videoPath = videoPath
        self.dirPaths = dirPaths
        self.frameCount = 0
        self.frameWidth = 0
        self.frameHeight = 0

        self.extractVideoFrames(slowdown)

    # 
    # Function extracts frames from video into output directory using ffmpeg
    #
    def extractVideoFrames(self, slowdown):
        retval = os.system(f'ffmpeg -i {self.videoPath} -vsync 0 {self.dirPaths["rFrames"]}/_{"%05d"}.png')
        if retval:
            print ("Error converting video file.")
            return
        
        # extract frame count
        self.frameCount = len(fnmatch.filter(os.listdir(self.dirPaths["rFrames"]), '*.png'))

        # set frame width and height by opening first file
        with open(self.dirPaths["rFrames"] + '/' + FrameReader.FIRST_FRAME_FILENAME, 'rb') as file:
            firstFrame = Image.open(file).convert('RGB')
            self.frameWidth, self.frameHeight = firstFrame.size

        # rename frame files to give space for intermediate frames
        index = 0
        for _, filename in enumerate(sorted(os.listdir(self.dirPaths["rFrames"]))):
            if filename.endswith('.png'):
                os.rename(self.dirPaths["rFrames"] + '/' + filename, self.dirPaths["iFrames"] + '/_' + str(index).zfill(5) + '.png')
                index += slowdown

    def getFrameCount(self):
        return self.frameCount

    def getWidth(self):
        return self.frameWidth

    def getHeight(self):
        return self.frameHeight  

    def getFramesByFirstIndex(self, firstFrameIndex):
        I0, I1 = None, None

        with open(self.framePath + '/' + '_%05d.png' % firstFrameIndex, 'rb') as file:
            I0 = Image.open(file).convert('RGB')
        with open(self.framePath + '/' + '_%05d.png' % firstFrameIndex + 1, 'rb') as file:
            I1 = Image.open(file).convert('RGB')

        return I0, I1
