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
        string input_path;
        int slowmo_factor;
        int output_fps;
        string output_path;
        unique_ptr<zmq::context_t> context;
        unique_ptr<zmq::socket_t> flow_requester;
        unique_ptr<FlowVectorFrame> buffer_frame;
        unique_ptr<VideoProcessor> video_processor;
        SlowMotionService(string input_path, int slowmo_factor, int output_fps, string output_path);
    public:
        SlowMotionService(SlowMotionService &other) = delete;
        void operator=(const SlowMotionService&) = delete;
        static SlowMotionService* GetInstance(string input_path, int slowmo_factor, int output_fps, string output_path);
        void startService();
};

#endif
