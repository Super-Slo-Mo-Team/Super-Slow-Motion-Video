#ifndef SLOW_MOTION_SERVICE_HPP
#define SLOW_MOTION_SERVICE_HPP

#include "flowVectorFrame.hpp"
#include "videoProcessor.hpp"

#include <zmq.hpp>
#include <memory>

using namespace std;

class SlowMotionService {
    private:
        static SlowMotionService* slowMotionService_;
        string inputPath;
        int slowmoFactor;
        int outputFps;
        string outputPath;
        torch::jit::script::Module interpolationModel;
        torch::jit::script::Module backWarpModel;
        torch::DeviceType device;
        zmq::context_t context;
        zmq::socket_t flowRequester;
        VideoProcessor* videoProcessor;
        FlowVectorFrame bufferFrame;
        SlowMotionService(string inputPath, int slowmoFactor, int outputFps, string outputPath);
    public:
        SlowMotionService(SlowMotionService &other) = delete;
        void operator=(const SlowMotionService&) = delete;
        static SlowMotionService* GetInstance(string inputPath, int slowmoFactor, int outputFps, string outputPath);
        void startService();
};

#endif
