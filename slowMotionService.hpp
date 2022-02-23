#ifndef SLOW_MOTION_SERVICE_HPP
#define SLOW_MOTION_SERVICE_HPP

#include "flowVectorFrame.hpp"

#include <zmq.hpp>
#include <memory>

using namespace std;

class SlowMotionService {
    private:
        string input_path;
        string slowmo_factor;
        string output_fps;
        string output_path;
        unique_ptr<zmq::context_t> context;
        unique_ptr<zmq::socket_t> flow_requester;
        unique_ptr<FlowVectorFrame> buffer_frame;
    public:
        SlowMotionService(string input_path, int slowmo_factor, int output_fps, string output_path);
        void startService();
};

#endif
