#ifndef FLOW_VECTOR_SERVICE_HPP
#define FLOW_VECTOR_SERVICE_HPP

#include "flowVectorFrame.hpp"

#include <zmq.hpp>
#include <string>
#include <memory>

using namespace std;

class FlowVectorService {
    private:
        static FlowVectorService* flowVectorService_;
        unique_ptr<zmq::context_t> context;
        unique_ptr<zmq::socket_t> flowRequester;
        unique_ptr<FlowVectorFrame> bufferFrame;
        void createFlowVectorFrame(int frameIndex, unique_ptr<FlowVectorFrame> &bufferFrame);
        FlowVectorService();
    public:
        FlowVectorService(FlowVectorService &other) = delete;
        void operator=(const FlowVectorService&) = delete;
        static FlowVectorService* GetInstance();
        void startService();
};

#endif
