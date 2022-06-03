#include "config.hpp"
#include "flowVectorService.hpp"
#include "slowMotionService.hpp"


#include <thread>
#include <iostream>
#include <direct.h>
#include <fstream>
//#include <windows.h>
#include <sstream>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;


int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout << "Incorrect number of arguments. Exiting." << endl;
        exit(EXIT_FAILURE);
    }
    
    //cout << current_path() << endl;
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

    if (slowmoFactor < 1) {
        cout << "Invalid slowmo factor. Exiting." << endl;
        exit(EXIT_FAILURE);
    }

    if (outputFps <= 0) {
        cout << "Invalid slowmo factor. Exiting." << endl;
        exit(EXIT_FAILURE);
    }

    // make all temp directories (ROOT_PATH, YUV_PATH, FLO_PATH)
    const boost::filesystem::path path(ROOT_PATH);
    boost::filesystem::remove_all(path);
    _mkdir(ROOT_PATH);
    _mkdir(YUV_PATH);
    _mkdir(OUT_PATH);
    _mkdir(FLO_PATH);
    

    // initialize services
    FlowVectorService *fvs = FlowVectorService::GetInstance();
    SlowMotionService *sms = SlowMotionService::GetInstance(inputPath, slowmoFactor, outputFps, outputPath);
    
    stringstream cudaCommand;
    cudaCommand << CUDA_EXEC << "--input=" << YUV_PATH << "\\*.yuv " << "--output=" << FLO_PATH << "\\";
    string cudaString = cudaCommand.str();

    system(cudaString.c_str());
   
    
    // create threads with appropriate entry points
    thread flowServiceThread(&FlowVectorService::startService, &*fvs);
    thread slowmoServiceThread(&SlowMotionService::startService, &*sms);
    

    // terminate program upon completion
    slowmoServiceThread.join();
    flowServiceThread.join();
    
     // remove all temp directories
    // TODO: need to delete files first too
    //_rmdir(ROOT_PATH);
    //_rmdir(YUV_PATH);
    //_rmdir(FLO_PATH);
    boost::filesystem::remove_all(path);

    return EXIT_SUCCESS;
}

