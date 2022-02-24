#ifndef FLOW_VECTOR_SERVICE_HPP
#define FLOW_VECTOR_SERVICE_HPP

#include "flowVectorFrame.hpp"

#include <zmq.hpp>
#include <string>
#include <memory>

using namespace std;

class FlowVectorService {
    private:
        unique_ptr<zmq::context_t> context;
        unique_ptr<zmq::socket_t> flow_requester;
        unique_ptr<FlowVectorFrame> buffer_frame;
        void createFlowVectorFrame(int frame_index, unique_ptr<FlowVectorFrame> &buffer_frame);
    public:
        FlowVectorService();
        void startService();
};

#endif
