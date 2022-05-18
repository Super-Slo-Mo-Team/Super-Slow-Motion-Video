#ifndef CONFIG_HPP
#define CONFIG_HPP

// config variables
#define MAX_FILE_DIGITS 5

// temp directories
#define ROOT_PATH "tmp"
#define YUV_PATH "tmp/yuv"
#define FLO_PATH "tmp/flo"

// model checkpoints location
#define MODEL_SCRIPT "../model/model.py"
#define FRAME_INTERPOLATION_MODEL_PATH "../model/checkpoints/FrameInterpolation.ckpt"
#define BACKWARP_MODEL_PATH "../model/checkpoints/traced_backwarp_model.pt"

// IPC comm
#define FV_SOCKET_PATH "ipc://tmp/flowVec.sock"
#define TERMINATION_MSG -1

#endif
