#ifndef CONFIG_HPP
#define CONFIG_HPP

// config variables
#define MAX_FILE_DIGITS 5

// temp directories
#define ROOT_PATH "tmp"
#define YUV_PATH "tmp\\yuv"
#define FLO_PATH "tmp\\flo"
#define OUT_PATH "tmp\\out"

#define CUDA_EXEC "..\\..\\..\\appOFCuda\\Release\\AppOFCuda.exe "


// model checkpoints location
#define MODEL_SCRIPT "..\\..\\..\\model\\model.py"
#define INTERPOLATION_CHECKPOINT_PATH "..\\..\\..\\model\\checkpoints\\epoch_23.ckpt"
#define INTERPOLATION_MODEL_PATH "..\\..\\..\\model\\checkpoints\\traced_frame_interpolation.ckpt"
#define BACKWARP_MODEL_PATH "..\\..\\..\\model\\checkpoints\\traced_backwarp_model.pt"

// IPC comm
#define FV_SOCKET_PATH "ipc://tmp/flowVec.sock"
#define TERMINATION_MSG -1

#endif
