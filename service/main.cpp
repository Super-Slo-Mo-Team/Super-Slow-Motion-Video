#include "config.hpp"
#include "flowVectorService.hpp"
#include "slowMotionService.hpp"

#include <thread>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout << "Incorrect number of arguments. Exiting." << endl;
        exit(EXIT_FAILURE);
    }

    // check input arguments
    string inputPath = argv[1];
    string outputPath = argv[2];
    int slowmoFactor, outputFps;

    try {
        slowmoFactor = stoi(argv[3]);
        outputFps = stoi(argv[4]);
    } catch (...) {
        cout << "Slowmo factor and/or output FPS are not integer numbers. Exiting." << endl;
        exit(EXIT_FAILURE);
    }

    if (slowmoFactor <= 1) {
        cout << "Invalid slowmo factor. Exiting." << endl;
        exit(EXIT_FAILURE);
    }

    if (outputFps <= 0) {
        cout << "Invalid slowmo factor. Exiting." << endl;
        exit(EXIT_FAILURE);
    }

    // TODO: make all temp directories (ROOT_PATH, YUV_PATH, FLO_PATH, R_FRAME_PATH, I_FRAME_PATH)

    // initialize services
    FlowVectorService *fvs = FlowVectorService::GetInstance();
    SlowMotionService *sms = SlowMotionService::GetInstance(inputPath, slowmoFactor, outputFps, outputPath);

    // create threads with appropriate entry points
    thread flowServiceThread(&FlowVectorService::startService, &*fvs);
    thread slowmoServiceThread(&SlowMotionService::startService, &*sms);
    
    // terminate program upon completion
    slowmoServiceThread.join();
    flowServiceThread.join();

    // TODO: remove all temp directories

    return EXIT_SUCCESS;
}
