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
#include <stdlib.h>


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
                pathBuilder << YUV_PATH << "/" << setfill('0') << setw(MAX_FILE_DIGITS) << frameCounter << ".yuv";
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

    if (!videoFrameCount) {
        cout << "Check valid file input." << endl;
    }

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

/**
 * @brief Get a pair of images transformed to tensors
 *
 * @param firstFrameIndex denoting the pair's first frame index
 */
vector<torch::Tensor> VideoProcessor::getFramePair(int frameIndex) {
    // given an index, pass the frame and its next (index + slowMoFactor)
    vector<torch::Tensor> framePair;

    stringstream pathBuilder1;
    stringstream pathBuilder2;
    pathBuilder1 << YUV_PATH << "/" << setfill('0') << setw(MAX_FILE_DIGITS) << frameIndex << ".yuv";
    pathBuilder2 << YUV_PATH << "/" << setfill('0') << setw(MAX_FILE_DIGITS) << frameIndex + this->slowmoFactor << ".yuv";
    string path1 = pathBuilder1.str();
    string path2 = pathBuilder2.str();

    torch::Tensor frame1 = this->fileToTensor(path1);
    torch::Tensor frame2 = this->fileToTensor(path2);

    framePair.push_back(frame1);
    framePair.push_back(frame2);

    return framePair;
}

/**
 * @brief facilitates the creation of a tensor from a YUV file
 *
 * @param file file path to .yuv file
 * @return torch::Tensor
 */
torch::Tensor VideoProcessor::fileToTensor(string file){
    vector<int> bytes = VideoProcessor::ReadAllBytes(file);

    int frameSize = this->videoHeight * this->videoWidth;

    vector<int>::const_iterator start = bytes.begin();
    vector<int>::const_iterator endOfY = bytes.begin() + frameSize;
    vector<int>::const_iterator endOfU = endOfY + frameSize/4;
    vector<int>::const_iterator endOfV = endOfU + frameSize/4;
    
    vector<int> yValues(start,endOfY);
    vector<int> uValues(endOfY,endOfU);
    vector<int> vValues(endOfU,endOfV);

    torch::Tensor Y = torch::empty(0,torch::kInt);
    torch::Tensor U = torch::empty(0,torch::kInt);
    torch::Tensor V = torch::empty(0,torch::kInt);

    yTensor(yValues, &Y);
    Y.unsqueeze_(0);

    mapColor(uValues, &U);
    U.unsqueeze_(0);

    mapColor(vValues, &V);
    V.unsqueeze_(0);
    
    if (Y.sizes() != U.sizes() || U.sizes() != V.sizes()) {
        cout << "YUV file formatted incorrectly, individual tensor sizes are not equal..." << endl;
        cout << "Y Tensor is of size: " << Y.sizes() << endl;
        cout << "U Tensor is of size: " << U.sizes() << endl;
        cout << "V Tensor is of size: " << V.sizes() << endl << endl;
        exit (EXIT_FAILURE);
    }

    torch::Tensor img = torch::empty(0,torch::kInt);
    img = torch::cat({img,Y,U,V},0);

    return img.unsqueeze(0);
}

/**
 * @brief reads the YUV file into a vector of int values
 *
 * @param filename file path to .yuv file
 * @return vector<int>
 */
vector<int> VideoProcessor::ReadAllBytes(string filename) {
    char * memblock;
    streampos size;
    ifstream ifs(filename, ios::binary|ios::ate);
    
    if (ifs.is_open()) {
        size = ifs.tellg();
        memblock = new char [size];
        ifs.seekg (0, ios::beg);
        ifs.read (memblock, size);
        ifs.close();
    } else {
      cout << "Unable to open file: " << filename << endl;
      return vector<int>(0);
    }

    vector<char> result(memblock, memblock+size);
    vector<int> intResult;

    for(int i = 0; i < size; i++) {
        intResult.push_back(int(result[i]));
    }

    return intResult;
}

/**
 * @brief maps the Y values into a tensor
 *
 * @param values Y values
 * @param yT Y Tensor
 */
void VideoProcessor::yTensor(std::vector<int> values, torch::Tensor* yT) {
    int* array = new int[values.size()];
    std::copy(values.begin(), values.end(), array);
    torch::Tensor yTensor = torch::from_blob(array,{this->videoHeight,this->videoWidth}, torch::kInt);
    *yT = torch::cat({*yT, yTensor},0);
}

/**
 * @brief maps color values into a tensor
 *
 * @param values color values
 * @param colorT color Tensor
 */
void VideoProcessor::mapColor(std::vector<int> values, torch::Tensor* colorT ) {
    torch::Tensor colorTensor = torch::empty(0,torch::kInt);
    torch::Tensor rowTensor = torch::empty(0,torch::kInt);
    
    for(int i = 0; i < values.size(); i++){
        torch::Tensor valueTensor = torch::full({2,2}, values[i], torch::kInt);
        rowTensor = torch::cat({rowTensor,valueTensor},1);

        if(rowTensor.sizes()[1] == this->videoWidth){
            colorTensor = torch::cat({colorTensor, rowTensor},0);
            rowTensor = torch::empty(0,torch::kInt);
        }
    }
    
    *colorT = torch::cat({*colorT, colorTensor},0);
    torch::Tensor unsignedTensorTransform = torch::full({this->videoHeight, this->videoWidth}, 128, torch::kInt);
    *colorT = torch::add(*colorT, unsignedTensorTransform);
}

/**
 * @brief takes the tensor, converts it back to a YUV file in the form of a vector<char>
 *
 * !! Can write this vector to a file by passing &vector[0] as the data
 *
 * @param img
 */
vector<char> VideoProcessor::tensorToYUV(torch::Tensor img) {
    img.squeeze_();

    auto Y = img[0];
    auto U = img[1];
    auto V = img[2];
    Y = Y.contiguous();
    
    std::vector<int> vectorY(Y.data_ptr<int>(), Y.data_ptr<int>() + Y.numel());
    std::vector<int> vectorU = this->resizeColorVector(U);
    std::vector<int> vectorV = this->resizeColorVector(V);

    vectorY.insert(vectorY.end(), vectorU.begin(), vectorU.end());
    vectorY.insert(vectorY.end(), vectorV.begin(), vectorV.end());

    // This could be optimized no? feel like theres a better way but havnt found it quite yet
    vector<char> yuvFile;
    for(int i = 0; i < vectorY.size(); i++) {
        yuvFile.push_back( char(vectorY[i]) );
    }

    return yuvFile;
}

/**
 * @brief shrinks color tensor into a vector 1/4 the size
 *
 * @param color
 */
vector<int> VideoProcessor::resizeColorVector(torch::Tensor color) {
    torch::Tensor unsignedTensorTransform = torch::full({this->videoHeight, this->videoWidth}, 128, torch::kInt);
    color = torch::sub(color,unsignedTensorTransform);
    vector<int> resizedVector;

    auto access = color.accessor<int,2>();
    for (int i = 0; i < videoHeight; i += 2) {
        for(int j = 0; j < videoWidth; j += 2 ) {
            resizedVector.push_back(access[i][j]);
        }
    }
    
    return resizedVector;
}
