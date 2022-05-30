import os
import sys
import subprocess
from subprocess import Popen
from config import *

def main():
    # generate flow vectors for training dataset
    for folder in sorted(os.listdir(TRAINING_TRAIN_PATH)):
        folderPath = os.path.join(TRAINING_TRAIN_PATH, folder)
        commands = []
        for filename in os.listdir(folderPath):
            file = filename.split(".")[-2]
            commands.append(f"ffmpeg -hide_banner -loglevel error -i  {folderPath}\\{file}.jpg -pix_fmt yuv420p {folderPath}\\{file}.yuv")

        processes = [Popen(cmd, shell=False) for cmd in commands]
        for p in processes: p.wait()
        yuvPath = os.path.join(folderPath, '*.yuv')
        subprocess.run([CUDA_COMMAND, f'--input={yuvPath}', f"--output={folderPath}/", "--subdirInput=True"])

    # generate flow vectors for validation dataset
    for folder in os.listdir(TRAINING_VALIDATE_PATH):
        folderPath = os.path.join(TRAINING_VALIDATE_PATH, folder)
        commands= []
        for filename in os.listdir(folderPath):
            file = filename.split(".")[-2]
            commands.append(f"ffmpeg -hide_banner -loglevel error -i {folderPath}\\{file}.jpg -pix_fmt yuv420p {folderPath}\\{file}.yuv")

        processes = [Popen(cmd, shell=False) for cmd in commands]
        for p in processes: p.wait()
        yuvPath = os.path.join(folderPath, '*.yuv')
        subprocess.run([CUDA_COMMAND, f'--input={yuvPath}', f"--output={folderPath}/", "--subdirInput=True"])

if __name__ == "__main__":
    main()
