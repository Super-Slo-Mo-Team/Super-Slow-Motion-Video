# Path Macros

TRAINING_ROOT_PATH          = './data.nosync'
VIDEO_DATASET_PATH          = './data.nosync/original_high_fps_videos'
TRAINING_TMP_PATH           = './data.nosync/tmp'
TRAINING_TRAIN_PATH         = './data.nosync/train'
TRAINING_TEST_PATH          = './data.nosync/test'
TRAINING_VALIDATE_PATH      = './data.nosync/validate'
TRAINING_CHECKPOINT_PATH    = './data.nosync/checkpoints'

# Training Macros

NUM_EPOCHS = 200            # number of epochs to train
TRAIN_BATCH_SIZE = 6        # batch size for training
VALIDATION_BATCH_SIZE = 10  # batch size for validation
LEARNING_RATE = 0.0001      # initial learning rate
MILESTONES = [100, 150]     # decrease learning rate at milestones by a factor of 0.1
CHECKPOINT_EPOCH = 5        # checkpoint saving frequency
NUM_ITERATIONS = 100        # progress iterations
