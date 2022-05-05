# Path Macros

TRAINING_ROOT_PATH          = 'data.nosync'
DATASET_FOLDER_NAME         = 'original_high_fps_videos'
VIDEO_DATASET_PATH          = TRAINING_ROOT_PATH + '/' + DATASET_FOLDER_NAME
TRAINING_TMP_PATH           = TRAINING_ROOT_PATH + '/tmp'
TRAINING_TRAIN_PATH         = TRAINING_ROOT_PATH + '/train'
TRAINING_TEST_PATH          = TRAINING_ROOT_PATH + '/test'
TRAINING_VALIDATE_PATH      = TRAINING_ROOT_PATH + '/validate'
TRAINING_CHECKPOINT_PATH    = 'checkpoints'

# Training Macros

NUM_EPOCHS = 200            # number of epochs to train
TRAIN_BATCH_SIZE = 6        # batch size for training
VALIDATION_BATCH_SIZE = 10  # batch size for validation
LEARNING_RATE = 0.0001      # initial learning rate
MILESTONES = [100, 150]     # decrease learning rate at milestones by a factor of 0.1
CHECKPOINT_EPOCH = 5        # checkpoint saving frequency
NUM_ITERATIONS = 100        # progress iterations
