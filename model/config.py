import os
import numpy as np
import torchvision.transforms as transforms

# CUDA command

CUDA_COMMAND = "C:\\Users\\SeanH\\Documents\\superSloMo\\Optical_Flow_SDK_3.0.15\\NvOFBasicSamples\\build\\AppOFCuda\\Debug\\AppOFCuda.exe"

# Dataset Path Macros

TRAINING_ROOT_PATH          = 'data.nosync'
DATASET_FOLDER_NAME         = 'original_high_fps_videos'
VIDEO_DATASET_PATH          = os.path.join(TRAINING_ROOT_PATH, DATASET_FOLDER_NAME)
TRAINING_TMP_PATH           = os.path.join(TRAINING_ROOT_PATH, 'tmp')
TRAINING_TRAIN_PATH         = os.path.join(TRAINING_ROOT_PATH, 'train')
TRAINING_TEST_PATH          = os.path.join(TRAINING_ROOT_PATH, 'test')
TRAINING_VALIDATE_PATH      = os.path.join(TRAINING_ROOT_PATH, 'validate')
TRAINING_CHECKPOINT_PATH    = 'checkpoints'

# Application Path Macros

INTRP_MODEL_PATH            =  os.path.join('..', 'model', TRAINING_CHECKPOINT_PATH, 'traced_frame_interpolation.ckpt')
BACKWARP_MODEL_PATH         =  os.path.join('..', 'model', TRAINING_CHECKPOINT_PATH, 'traced_backwarp_model.pt')

# Dimensions

IMAGE_DIM = (640, 360)                  # dataset image dimensions
TRAIN_RANDOM_CROP_SIZE = (352, 352)     # training random crop size
VALIDATE_RANDOM_CROP_SIZE = (640, 352)  # validation random crop size

# Misc

TSTEPS = np.linspace(0.125, 0.875, 7)   # array of timesteps for intermediate frame calculations
FRAME_TRANSFORM = transforms.Compose([  # transformation to apply to images from the adobe240fps dataset
    transforms.ToTensor(),
    transforms.Normalize(
        mean=[0.429, 0.431, 0.397],
        std=[1, 1, 1]
    )
])

# Training Macros

NUM_EPOCHS = 200                        # number of epochs to train
TRAIN_BATCH_SIZE = 6                    # batch size for training
VALIDATION_BATCH_SIZE = 10              # batch size for validation
LEARNING_RATE = 0.0001                  # initial learning rate
MILESTONES = [100, 150]                 # decrease learning rate at milestones by a factor of 0.1
CHECKPOINT_EPOCH = 5                    # checkpoint saving frequency
NUM_ITERATIONS = 100                    # progress iterations
