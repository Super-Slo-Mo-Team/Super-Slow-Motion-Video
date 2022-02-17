#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <typeinfo>

#include <zmq.hpp>
#include <zhelpers.hpp>

#include "FlowVectors.h"

#define RESPONSE_SUCCESS 0
#define RESPONSE_FAILURE 1

using namespace std;


/**
 * @brief Construct a new Flow Frame::Flow Frame object
 *
 * @param file
 * @param flowFrameIndex
 */
FlowFrame::FlowFrame(istream& file, int flowFrameIndex) {
    this->flowFrameIndex = flowFrameIndex;
    float dummy;

    file.read(reinterpret_cast<char*>(&dummy), sizeof(float));
    file.read(reinterpret_cast<char*>(&this->width), sizeof(int));
    file.read(reinterpret_cast<char*>(&this->height), sizeof(int));

    for (int i = 0; i != this->width * this->height; i++) {
        float x, y;
        file.read(reinterpret_cast<char*>(&x), sizeof(float));
        file.read(reinterpret_cast<char*>(&y), sizeof(float));

        this->xFlow << ',' << x;
        this->yFlow << ',' << y;
    }
}

/**
 * @brief Get a frame's index
 */
int FlowFrame::getFlowFrameIndex() {
    return this->flowFrameIndex;
}

/**
 * @brief Return a frame's xFlow
 */
int FlowFrame::getWidth() {
    return this->width;
}

/**
 * @brief Return a frame's xFlow
 */
int FlowFrame::getHeight() {
    return this->height;
}

/**
 * @brief Return a frame's xFlow
 */
string FlowFrame::getXFlow() {
    return this->xFlow.str();
}

/**
 * @brief Return a frame's yFlow
 */
string FlowFrame::getYFlow() {
    return this->yFlow.str();
}


/**
 * @brief Process video by constructing frame objects and sending information through IPC
 *   
 * @param flowPath
 */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Incorrect invocation. Exiting." << endl;
        exit(EXIT_FAILURE);
    }

    // create context
    zmq::context_t context(1);

    // initialize requester socket
    zmq::socket_t flowRequester(context, ZMQ_REQ);
    cout << "Connecting client to localhost server on port 8080..." << endl;
	flowRequester.connect("tcp://localhost:8080");

    // preprocess
    string flowPath = argv[1];
    int flowFrameIndex = 0;

    while (1) {
        // build file path
        stringstream pathBuilder;
        pathBuilder << flowPath << "/_" << setfill('0') << setw(5) << flowFrameIndex << ".flo";
        string filename = pathBuilder.str();

        // open file
        ifstream file(filename, std::ifstream::binary);

        // end of flo files
        if (!file) {
            if (!flowFrameIndex) {
                cout << "No .flo files in directory: " << flowPath << ". Exiting." << endl;
                exit(EXIT_FAILURE);
            } else {
                break;
            }
        }

        // create FlowFrame from file
        cout << "CPP: Reading file:" << filename << endl;
        FlowFrame flowFrame(file, flowFrameIndex);

        // close file
        file.close();

        // idx,width,height,x1,x2,...,xn:y1,y2,...,yn
        stringstream metadata;
        metadata << flowFrame.getFlowFrameIndex() << ',' << flowFrame.getWidth()
            << ',' << flowFrame.getHeight();
        string msg = metadata.str() + ':' + flowFrame.getXFlow() + ':' + flowFrame.getYFlow();
        
        // send flow data to socket and wait for response
        s_send(flowRequester, msg);
        int res = stoi(s_recv(flowRequester));

        // continue iteration or retry
        if (res == RESPONSE_SUCCESS) {
            cout << "CPP: Received Success reading " << flowFrame.getFlowFrameIndex() << endl;
            flowFrameIndex++;
        } else if (res != RESPONSE_FAILURE) {
            cout << "CPP: Received Failure reading " << flowFrame.getFlowFrameIndex() << endl;
            continue;
        }
    }
}
