#include "FlowVectors.h"


#include <iostream>
#include <fstream>
#include <typeinfo>
#include <sstream>


using namespace std;

/**
 * @brief Construct a new Flow Video:: Flow Video object
 *        Iterates over files in given path that fit the middlebury.flo naming
 *        Creates flow Frames from each which are added to a vector of frames: flowFrames
 * @param outPath 
 */

FlowVideo::FlowVideo(string outPath){
    frameIndx = 0;
    

    while(true){
       stringstream pathBuilder;
       pathBuilder << outPath << "/" << "_" << setfill('0') << setw(5) << frameIndx << "_" << "middlebury" << ".flo"; 
       string flowFile = pathBuilder.str();
       ifstream file(flowFile, std::ifstream::binary);
       if (file){
           cout << "Reading file:" << flowFile << endl;
           FlowFrame current(file, frameIndx);
           flowFrames.push_back(current);
           file.close();
           frameIndx++;
       } else {
           break;
       }
   }

   if (frameIndx == 0){ //no file was ever opened
        cout << "No .flo files in directory:" << outPath << endl;
        exit(EXIT_FAILURE);
    }
}
/**
 * @brief returns the flow frame vector
 * 
 * @return vector<FlowFrame> 
 */
vector<FlowFrame> FlowVideo::getFlowFrames(){
    return flowFrames;
}


/**
 * @brief Construct a new Flow Frame:: Flow Frame object
 *   
 * @param flowFile 
 * @param frameI 
 */

FlowFrame::FlowFrame(istream& flowFile, int frameI){
    frameNumber = frameI;
    float test;

    flowFile.read(reinterpret_cast<char*>(&test), sizeof(float));
    flowFile.read(reinterpret_cast<char*>(&width), sizeof(int));
    flowFile.read(reinterpret_cast<char*>(&height), sizeof(int));

    //cout << test << " " << height << " " << width << endl;

    while (true){
        if(flowFile.eof()){
            break;
        }
        float x;
        float y;
        flowFile.read(reinterpret_cast<char*>(&x), sizeof(float));
        flowFile.read(reinterpret_cast<char*>(&y), sizeof(float));

        xFlow.push_back(x);
        yFlow.push_back(y);
    }


    //testing constructor
    /*
    cout << xFlow.at(0) << " " << yFlow.at(0) << endl;
    cout << xFlow.at(1) << " " << yFlow.at(1) << endl;
    cout << xFlow.at(2) << " " << yFlow.at(2) << endl;
    cout << xFlow.at(100000) << " " << yFlow.at(100000) << endl;
    cout << xFlow.at(230398) << " " << yFlow.at(230398) << endl;
    cout << xFlow.at(230399) << " " << yFlow.at(230399) << endl;
    cout << xFlow.at(230400) << " " << yFlow.at(230400) << endl;
    cout << xFlow.back() << " " << yFlow.back() << endl;
    cout << xFlow.size() << " " << yFlow.size() << endl;// 1 more pixel than what we should? should be 230400, but get 230401
    cout << "Done" << endl;
    */


}

vector<float> FlowFrame::getXFlow(){
    return xFlow;
}
vector<float> FlowFrame::getYFlow(){
    return yFlow;
}

int FlowFrame::getFrameIndex(){
    return frameNumber;
}

/**
 * @brief just shows some usage of these classes
 * 
 * 
 * 
 * 
 */
int main(int argc, char *argv[]){
    FlowVideo test("./flowFiles"); //this is the directory name for me locally

    vector<FlowFrame> frames = test.getFlowFrames();
    FlowFrame first = frames.at(0);//gets first 
    cout << "First 5 pixels of frame: " << first.getFrameIndex() << endl; 
    for(int i = 0; i < 5; i++){
        cout << "x vector: " << first.getXFlow().at(i) << "  y vector:" << first.getYFlow().at(i) << endl;
    }
    
}