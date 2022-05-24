import os
import subprocess
from config import *

def main():
    # generate flow vectors for training dataset
    for folder in os.listdir(TRAINING_TRAIN_PATH):
        folderPath = os.path.join(TRAINING_TRAIN_PATH, folder)
        yuvPath = os.path.join(folderPath, '*.yuv')
        subprocess.run([CUDA_COMMAND, f'--input={yuvPath}', f"--output={folderPath}/", "--subdirInput=True"])

    # generate flow vectors for validation dataset
    for folder in os.listdir(TRAINING_VALIDATE_PATH):
        folderPath = os.path.join(TRAINING_VALIDATE_PATH, folder)
        yuvPath = os.path.join(folderPath, '*.yuv')
        subprocess.run([CUDA_COMMAND, f'--input={yuvPath}', f"--output={folderPath}/", "--subdirInput=True"])

if __name__ == "__main__":
    main()
