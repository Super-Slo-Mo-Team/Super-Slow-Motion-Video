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
        unique_ptr<zmq::context_t> context;
        unique_ptr<zmq::socket_t> flowRequester;
        unique_ptr<FlowVectorFrame> bufferFrame;
        unique_ptr<VideoProcessor> videoProcessor;
        SlowMotionService(string inputPath, int slowmoFactor, int outputFps, string outputPath);
    public:
        SlowMotionService(SlowMotionService &other) = delete;
        void operator=(const SlowMotionService&) = delete;
        static SlowMotionService* GetInstance(string inputPath, int slowmoFactor, int outputFps, string outputPath);
        void startService();
};

#endif
