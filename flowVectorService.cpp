#include "flowVectorService.hpp"
#include "config.hpp"

#include <zhelpers.hpp>
#include <iostream>
#include <fstream>

using namespace std;

/**
 * @brief Create a FlowVectorService::FlowVectorService object to handle returning
 * FlowVectorFrame objects parsed from flow files over TCP
 */
FlowVectorService::FlowVectorService() {
    // create context
    this->context = make_unique<zmq::context_t>(1);

    // initialize requester socket on localhost:8080
    this->flow_requester = make_unique<zmq::socket_t>(*context, ZMQ_REP);
    cout << "FVS: Binding responder to tcp://127.0.0.1:5555..." << endl;
	flow_requester->bind("tcp://127.0.0.1:5555");
}

/**
 * @brief Create a forever loop listening for requests to load a specific flow frame
 */
void FlowVectorService::startService() {
    while (1) {
        // listen to incoming requests
        int frame_index = stoi(s_recv(*flow_requester));

        // output received request
        if (frame_index == -1) {
            cout << "FVS: Received request to break " << endl;
            break;
        } else {
            cout << "FVS: Received request to read frame " << frame_index << endl;
        }
        
        // generate flow frame to be sent over IPC
        createFlowVectorFrame(frame_index, buffer_frame);
        
        // serialize and send buffer_frame
        stringstream msg;
        boost::archive::text_oarchive serializer(msg);
        serializer << *buffer_frame.get();
        string serialized_msg = msg.str();

        // send flow data to socket
        s_send(*flow_requester, serialized_msg);
    }
}

/**
 * @brief Construct a frame object to be sent through IPC
 *   
 * @param frame_index denoting which flow frame to retrieve
 * @param buffer_frame containing reference to flowFrame object where information will be loaded
 */
void FlowVectorService::createFlowVectorFrame(int frame_index, unique_ptr<FlowVectorFrame> &buffer_frame) {
    // build file path using frame_index
    stringstream path_builder;
    path_builder << FLO_PATH << "/_" << setfill('0') << setw(5) << frame_index << ".flo";
    string filename = path_builder.str();

    // open file
    ifstream file = ifstream(filename, ifstream::binary);

    // error
    if (!file) {
        cout << "FVS: No flo file with index " << frame_index << " in directory: " << FLO_PATH << ". Exiting." << endl;
        exit(EXIT_FAILURE);
    }

    // create FlowFrame from file
    buffer_frame = make_unique<FlowVectorFrame>(file, frame_index);

    // close file
    file.close();
}
