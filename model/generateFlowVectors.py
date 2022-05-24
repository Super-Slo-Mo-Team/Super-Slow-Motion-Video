import os
from config import *

def main():
    # generate flow vectors for training dataset
    for folder in os.listdir(TRAINING_TRAIN_PATH):
        os.system('../appOFCuda/[executable name] ', os.path.join(TRAINING_TRAIN_PATH, folder))

    # generate flow vectors for validation dataset
    for folder in os.listdir(TRAINING_VALIDATE_PATH):
        os.system('../appOFCuda/[executable name] ', os.path.join(TRAINING_VALIDATE_PATH, folder))

if __name__ == "__main__":
    main()
