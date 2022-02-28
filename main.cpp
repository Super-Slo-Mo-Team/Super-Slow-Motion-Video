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

    string inputPath = argv[1];
    int slowmoFactor = stoi(argv[2]);
    int outputFps = stoi(argv[3]);
    string outputPath = argv[4];

    // initialize services
    FlowVectorService *fvs = FlowVectorService::GetInstance();
    SlowMotionService *sms = SlowMotionService::GetInstance(inputPath, slowmoFactor, outputFps, outputPath);

    // create threads with appropriate entry points
    thread flowServiceThread(&FlowVectorService::startService, &*fvs);
    thread slowmoServiceThread(&SlowMotionService::startService, &*sms);
    
    // terminate program upon completion
    slowmoServiceThread.join();
    flowServiceThread.join();

    return EXIT_SUCCESS;
}
