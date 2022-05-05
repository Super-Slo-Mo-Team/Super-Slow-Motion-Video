from shutil import move, rmtree
import os
import random
from config import *

# generates samples of 12 frames from videos
def generateClips(videos, indices, outputPath):
    # split training videos into clips of 12 frames into train folder
    os.mkdir(outputPath)
    clipCounter = 0
    for i in indices:
        videoPath = os.path.join(TRAINING_TMP_PATH, videos[i][:-4])
        frames = sorted(os.listdir(videoPath))
        clips = [clip for clip in [frames[p:p + 12] for p in range(0, len(frames), 12)] if len(clip) == 12]

        for clip in clips:
            os.mkdir(os.path.join(outputPath, str(clipCounter)))
            for frame in clip:
                move(
                    "{}/{}".format(videoPath, frame),
                    "{}/{}/{}".format(outputPath, clipCounter, frame)
                )

            clipCounter += 1

def main():
    # get list of dataset videos
    videos = [video for video in os.listdir(VIDEO_DATASET_PATH) if 
        video.lower().endswith('.mov') or
        video.lower().endswith('.mp4') or
        video.lower().endswith('.m4v')]

    # generate frames per video in frame folders with video name
    os.mkdir(TRAINING_TMP_PATH)
    for video in videos:
        videoName = video[:-4]
        framePath = os.path.join(TRAINING_TMP_PATH, videoName)

        os.mkdir(framePath)
        retval = os.system(
            'ffmpeg -i {} -vf scale={}:{} -vsync 0 -qscale:v 2 {}/%04d.jpg'.format(
                os.path.join(VIDEO_DATASET_PATH, video),
                640,
                360,
                framePath
            )
        )
        if retval:
            print ("Error converting file: {}.".format(video))

    # make a 90 : 10 train test split
    videoIndices = list(range(len(videos)))
    testSampleIndices = sorted(random.sample(videoIndices, len(videos) // 10))
    trainSampleIndices = [i for i in videoIndices if i not in testSampleIndices]

    # generate clips of 12 frames for both training and test sets
    generateClips(videos, trainSampleIndices, TRAINING_TRAIN_PATH)
    generateClips(videos, testSampleIndices, TRAINING_TEST_PATH)
    
    # sample 100 clips for validation set
    os.mkdir(TRAINING_VALIDATE_PATH)
    indices = sorted(random.sample(range(len(os.listdir(TRAINING_TEST_PATH))), 100))
    for index in indices:
        move("{}/{}".format(TRAINING_TEST_PATH, index), "{}/{}".format(TRAINING_VALIDATE_PATH, index))

    # create directory for training checkpoints
    os.mkdir(TRAINING_CHECKPOINT_PATH)

    # TODO: remove unnecessary data

if __name__ == "__main__":
    main()
