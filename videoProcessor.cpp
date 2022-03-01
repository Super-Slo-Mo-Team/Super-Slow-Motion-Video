#include "videoProcessor.hpp"
#include "config.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctype.h>
#include <cstring>
#include <vector>
#include <iterator>

using namespace std;

// singleton class instance
VideoProcessor *VideoProcessor::videoProcessor_ = nullptr;

/**
 * @brief Create a singleton instance of FlowVectorService::FlowVectorService class
 */
VideoProcessor* VideoProcessor::GetInstance(string inputPath, int slowmoFactor) {
    if (videoProcessor_ == nullptr) {
        videoProcessor_ = new VideoProcessor(inputPath, slowmoFactor);
    }

    return videoProcessor_;
}

/**
 * @brief Construct a new VideoProcessor::VideoProcessor object
 *
 * @param inputPath specifying path to input video file
 * @param slowmoFactor specifying how much the input video needs to be slowed down by
 */
VideoProcessor::VideoProcessor(string inputPath, int slowmoFactor) {
    this->inputPath = inputPath;
    this->slowmoFactor = slowmoFactor;
    this->videoFrameCount = 0;
    this->videoWidth = 0;
    this->videoHeight = 0;

    extractVideoFrames();
}

/**
 * @brief Extract frames in yuv format from input video
 *
 */
void VideoProcessor::extractVideoFrames() {
    // open input file
    ifstream file;
    file.open(inputPath, ios::binary);

    char ch = 0;
    char *chBuffer = nullptr;
    bool foundW = false, foundH = false;
    int frameCounter = 0;

    // read file one char at a time
    while (file.get(ch)) {
        if ((ch) == 'W' && !foundW) {
            string width = "";
            file.get(ch);
            while (isdigit(ch)) {
                width += (ch);
                file.get(ch);
            }
            videoWidth = stoi(width);
            cout << "Video width: " << videoWidth << endl;
            foundW = true;
        }

        if ((ch) == 'H' && !foundH) {
            string height = "";
            file.get(ch);
            while (isdigit(ch)) {
                height += (ch);
                file.get(ch);
            }
            videoHeight = stoi(height);
            cout << "Video height: " << videoHeight << endl;
            foundH = true;
        }

        if (ch == 'F') {
            string cur = "F";
            int j = 0;
            while (file.get(ch) && j < 4) {
                cur += ch;
                j++;
            }
            
            if (cur.compare("FRAME") == 0) {
                videoFrameCount++;
                int size = videoWidth * videoHeight * 3 / 2;
                
                if (chBuffer == nullptr) {
                    chBuffer = (char*)malloc(size);
                }
                
                // read frame
                file.read(chBuffer, size);
                
                // create output file path
                stringstream pathBuilder;
                pathBuilder << YUV_PATH << "/_" << setfill('0') << setw(5) << frameCounter << ".yuv";
                string filename = pathBuilder.str();

                // output file
                ofstream output;
                output.open(filename, ios::binary);
                ostream_iterator<unsigned char> it(output);
                output.write(chBuffer, size);
                output.close();
                
                // increment frame counter
                frameCounter += slowmoFactor;
            }
        }
    }

    file.close();
    cout << "Frames contained in video: " << videoFrameCount << endl;

    if (chBuffer != nullptr) {
        free(chBuffer);
    }
}

/**
 * @brief Get the total number of frames in the video
 */
int VideoProcessor::getVideoFrameCount() {
    return videoFrameCount;
}

/**
 * @brief Get the video width
 */
int VideoProcessor::getVideoWidth() {
    return videoWidth;
}

/**
 * @brief Get the video height
 */
int VideoProcessor::getVideoHeight() {
    return videoHeight;
}

// TODO
/**
 * @brief Get a pair of images transformed to tensors
 * 
 * @param firstFrameIndex denoting the pair's first frame index
 */
int getFramePair(int firstFrameIndex, int secondIndex) {
//     // I0, I1 = None, None

//     // with open(dirPaths["iFrames"] + '/_%05d.png' % firstFrameIndex, 'rb') as file:
//     //     I0 = Image.open(file).convert('RGB')
//     // with open(dirPaths["iFrames"] + '/_%05d.png' % (firstFrameIndex + self.slowdown), 'rb') as file:
//     //     I1 = Image.open(file).convert('RGB')

//     // I0 = transforms.ToTensor()(I0).unsqueeze_(0)
//     // I1 = transforms.ToTensor()(I1).unsqueeze_(0)

//     // return I0, I1
    return 0;
}
