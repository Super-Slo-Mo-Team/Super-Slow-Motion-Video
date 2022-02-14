#include "FlowVectors.h"


#include <iostream>
#include <fstream>
#include <typeinfo>
#include <sstream>


using namespace std;

/**
 * @brief Construct a new Flow Frame::Flow Frame object
 *
 * @param flowFile
 * @param frameIndex
 */
FlowFrame::FlowFrame(istream& flowFile, int frameIndex) {
    this->frameIndex = frameIndex;
    float dummy;

    flowFile.read(reinterpret_cast<char*>(&dummy), sizeof(float));
    flowFile.read(reinterpret_cast<char*>(&this->width), sizeof(int));
    flowFile.read(reinterpret_cast<char*>(&this->height), sizeof(int));

    for(int i = 0; i != this->width * this->height; i++){
        float x, y;
        flowFile.read(reinterpret_cast<char*>(&x), sizeof(float));
        flowFile.read(reinterpret_cast<char*>(&y), sizeof(float));

        this->xFlow.push_back(x);
        this->yFlow.push_back(y);
    }
    cout << xFlow.size() << endl;
}

/**
 * @brief Get a frame's index
 */
int FlowFrame::getFrameIndex() {
    return this->frameIndex;
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
vector<float> FlowFrame::getXFlow() {
    return this->xFlow;
}


vector<float> FlowFrame::getYFlow() {
    return this->yFlow;
}

/**
 * @brief Construct a new Flow Video::Flow Video object
 *        Iterates over files in given path that fit the middlebury.flo naming
 *        Creates flow Frames from each which are added to a vector of frames: flowFrames
 * @param floDir
 */
FlowVideo::FlowVideo(string floDir){
    int frameIndex = 0;

    while (true) {
       stringstream pathBuilder;
       pathBuilder << floDir << "/" << "_" << setfill('0') << setw(5) << frameIndex << "_" << "middlebury" << ".flo";
       string flowFile = pathBuilder.str();
       ifstream file(flowFile, std::ifstream::binary);
       if (file) {
           cout << "Reading file:" << flowFile << endl;
           this->flowFrames.push_back(FlowFrame(file, frameIndex));
           file.close();
           frameIndex++;
       } else {
           break;
       }
   }

    // no file was opened
   if (!frameIndex) {
        cout << "No .flo files in directory:" << floDir << endl;
        exit(EXIT_FAILURE);
    }
}

vector<FlowFrame> FlowVideo::getFlowFrames() {
    return this->flowFrames;
}


/**
 * @brief Construct a new Flow Frame:: Flow Frame object
 *   
 * @param flowFile 
 * @param frameI 
 */
int main(int argc, char *argv[]) {
    // create FlowVideo object
    FlowVideo flowVideo = FlowVideo("./flowFiles");

    // retrieve first frame from frames vector
    vector<FlowFrame> frames = flowVideo.getFlowFrames();

    // output information into file with the following format:
    // width height
    // x vector
    // y vector
    // ...
    // where x's amd y's are separated by spaced
    // and vectors are separated by newlines
    ofstream output;
    stringstream pathBuilder;
    pathBuilder << "output" << "_flow.txt";
    string outputFilename = pathBuilder.str();
    output.open(outputFilename);

    // write width and height to file
    FlowFrame first = frames.at(0);
    output << first.getWidth() << ' ' << first.getHeight() << ' ' << frames.size() << '\n';

    // go through each frame and write to file
    for (vector<FlowFrame>::iterator it = begin(frames); it != end(frames); ++it) {
        vector<float> xFlow = (*it).getXFlow();
        for (int i = 0; i < xFlow.size(); ++i) {
            output << xFlow[i] << ' ';
        }
        output << '\n';
        
        vector<float> yFlow = (*it).getYFlow();
        for (int i = 0; i < yFlow.size(); ++i) {
            output << xFlow[i] << ' ';
        }
        output << '\n';
    }

