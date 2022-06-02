#ifndef FLOW_VECTOR_SERVICE_HPP
#define FLOW_VECTOR_SERVICE_HPP

#include "flowVectorFrame.hpp"

#define ZMQ_STATIC
#include "zmq.hpp"
#include <string>
#include <memory>

using namespace std;

class FlowVectorService {
    private:
        static FlowVectorService* flowVectorService_;
        zmq::context_t context;
        zmq::socket_t flowRequester;
        FlowVectorFrame bufferFrame;
        void createFlowVectorFrame(int frameIndex);
        FlowVectorService();
    public:
        FlowVectorService(FlowVectorService &other) = delete;
        void operator=(const FlowVectorService&) = delete;
        static FlowVectorService* GetInstance();
        void startService();
};

#endif
