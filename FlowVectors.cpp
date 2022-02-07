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

FlowFrame FlowVideo::getFrame(int index){
    return flowFrames.at(index);
}


/**
 * @brief Construct a new Flow Frame:: Flow Frame object
 *   
 * @param flowFile 
 * @param frameI 
 */

FlowFrame::FlowFrame(istream& flowFile, int frameI){
    frameNumber = frameI; //might not need this for anything
    float test;
    
    flowFile.read(reinterpret_cast<char*>(&test), sizeof(int));
    flowFile.read(reinterpret_cast<char*>(&width), sizeof(int));
    flowFile.read(reinterpret_cast<char*>(&height), sizeof(int));

    //cout << test << " " << height << " " << width << endl;

   for(int i = 0; i < height; i++){
       vector<Pixel> row;
       for (int j = 0; j < width; j++){
            float x;
            float y;
            flowFile.read(reinterpret_cast<char*>(&x), sizeof(float));
            flowFile.read(reinterpret_cast<char*>(&y), sizeof(float));
            Pixel p(x,y);
            row.push_back(p);
       }
    flowVectors.push_back(row);
   }


}

/*Not sure what the indexing of thie pixels is coming from the .flo files, could be:
    [(0,2) (1,2) (2,2)]        [(0,0) (1,0) (2,0)]
    [(0,1) (1,1) (2,1)]   or   [(0,1) (1,1) (2,1)]  
    [(0,0) (1,0) (2,0)]        [(0,2) (1,2) (2,2)]   
*/
Pixel FlowFrame::getPixelAt(int y, int x){
    return flowVectors[y][x];
}


int FlowFrame::getFrameIndex(){
    return frameNumber;
}


Pixel::Pixel(float xIn, float yIn){
    xVec = xIn;
    yVec = yIn;
}

float Pixel::x(){
    return xVec;
}
float Pixel::y(){
    return yVec;
}

/**
 * @brief just shows some usage of these classes
 * 
 * 
 */
int main(int argc, char *argv[]){
    FlowVideo test("./flowFiles"); //this is the directory name for me locally
    FlowFrame frame = test.getFrame(0);//gets first 
    //get each frame with frame.at()
    //while doing work with that frame we want to iterate over pixels
    //p = getPixelAt(x,y), might be useful to build a kernel of pixels with an average flow vector?
    //p.x() & p.y()
    Pixel p = frame.getPixelAt(100,300);
    cout << p.x() << " , " << p.y() << endl;
    
}