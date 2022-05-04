#ifndef CONFIG_HPP
#define CONFIG_HPP

// config variables
#define MAX_FILE_DIGITS 5

// temp directories
#define ROOT_PATH "./tmp"
#define YUV_PATH "./tmp/yuv"
#define FLO_PATH "./tmp/flo"
#define R_FRAME_PATH "./tmp/rFrames"
#define I_FRAME_PATH "./tmp/iFrames"

// IPC comm
#define FV_SOCKET_PATH "ipc://tmp/flowVec.sock"
#define TERMINATION_MSG -1

#endif
