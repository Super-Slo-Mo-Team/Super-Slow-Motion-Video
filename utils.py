from PIL import Image
import os
import fnmatch


class FrameReader():
    FIRST_FRAME_FILENAME = '_00001.png'

    def __init__(self, videoPath, framePath):
        self.videoPath = videoPath
        self.framePath = framePath if framePath else '.'
        self.frameCount = 0
        self.frameWidth = 0
        self.frameHeight = 0

        self.extractVideoFrames()

    # 
    # Function extracts frames from video into output directory using ffmpeg
    #
    def extractVideoFrames(self):
        retval = os.system('ffmpeg -i {} -vsync 0 {}/_%05d.png'.format(self.videoPath, self.framePath))
        if retval:
            print ("Error converting video file.")
            return
        
        # extract frame count
        self.frameCount = len(fnmatch.filter(os.listdir(self.framePath), '*.png'))

        # set frame width and height by opening first file
        with open(self.framePath + '/' + FrameReader.FIRST_FRAME_FILENAME, 'rb') as file:
            firstFrame = Image.open(file).convert('RGB')
            self.frameWidth, self.frameHeight = firstFrame.size

    def getFrameCount(self):
        return self.frameCount

    def getWidth(self):
        return self.frameWidth

    def getHeight(self):
        return self.frameHeight  

    # TODO: implement
    def getFramesByFirstIndex(self, firstFrameIndex):
        return None, None
