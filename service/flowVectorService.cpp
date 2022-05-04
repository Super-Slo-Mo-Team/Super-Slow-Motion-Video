#include "flowVectorService.hpp"
#include "config.hpp"

#include <zhelpers.hpp>
#include <iostream>
#include <fstream>

using namespace std;

// singleton class instance
FlowVectorService *FlowVectorService::flowVectorService_ = nullptr;

/**
 * @brief Create a singleton instance of FlowVectorService::FlowVectorService class
 */
FlowVectorService* FlowVectorService::GetInstance() {
    if (flowVectorService_ == nullptr) {
        flowVectorService_ = new FlowVectorService();
    }

    return flowVectorService_;
}

/**
 * @brief Create a FlowVectorService::FlowVectorService object to handle returning
 * FlowVectorFrame objects parsed from flow files over TCP
 */
FlowVectorService::FlowVectorService() {
    // create context
    context = zmq::context_t(1);

    // initialize requester socket on localhost:8080
    flowRequester = zmq::socket_t(context, ZMQ_REP);
    cout << "FVS: Binding responder to " << FV_SOCKET_PATH << "..." << endl;
	flowRequester.bind(FV_SOCKET_PATH);
}

/**
 * @brief Create a forever loop listening for requests to load a specific flow frame
 */
void FlowVectorService::startService() {
    while (1) {
        // listen to incoming requests
        int frameIndex = stoi(s_recv(flowRequester));

        // output received request
        if (frameIndex == TERMINATION_MSG) {
            cout << "FVS: Received request to break " << endl;
            break;
        } else {
            cout << "FVS: Received request to read frame " << frameIndex << endl;
        }
        
        // generate flow frame to be sent over IPC
        createFlowVectorFrame(frameIndex);
        
        // serialize and send buffer_frame
        stringstream msg;
        boost::archive::binary_oarchive serializer(msg);
        serializer << bufferFrame;
        string serializedMsg = msg.str();

        // send flow data to socket
        s_send(flowRequester, serializedMsg);
    }
}

/**
 * @brief Construct a frame object to be sent through IPC
 *   
 * @param frameIndex denoting which flow frame to retrieve
 */
void FlowVectorService::createFlowVectorFrame(int frameIndex) {
    // build file path using frame_index
    stringstream pathBuilder;
    pathBuilder << FLO_PATH << "/_" << setfill('0') << setw(MAX_FILE_DIGITS) << frameIndex << ".flo";
    string filename = pathBuilder.str();

    // open file
    ifstream file = ifstream(filename, ifstream::binary);

    // error
    if (!file) {
        cout << "FVS: No flo file with index " << frameIndex << " in directory: " << FLO_PATH << ". Exiting." << endl;
        exit(EXIT_FAILURE);
    }

    // create FlowFrame from file
    bufferFrame.readFloFile(file, frameIndex);

    // close file
    file.close();
}
