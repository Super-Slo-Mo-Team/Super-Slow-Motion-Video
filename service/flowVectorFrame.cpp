#include "flowVectorFrame.hpp"

#include <iostream>

using namespace std;

/**
 * @brief Allocate float pointers for the FlowVectorFrame::FlowVectorFrame object
 */
void FlowVectorFrame::setup() {
    xFlowFor = new float[numVecs];
    yFlowFor = new float[numVecs];
    xFlowBack = new float[numVecs];
    yFlowBack = new float[numVecs];
}

/**
 * @brief Load a frame's info into FlowVectorFrame::FlowVectorFrame object
 *
 * @param file1 containing unparsed forward flow vector information
 * @param file2 containing unparsed backward flow vector information
 * @param frameIndex denoting the frame
 */
void FlowVectorFrame::readFloFile(istream& file1, istream& file2, int frameIndex) {
    this->frameIndex = frameIndex;
    
    float dummy, x, y;
    int width1, height1, width2, height2;
    file1.read(reinterpret_cast<char*>(&dummy), sizeof(float));
    file1.read(reinterpret_cast<char*>(&width1), sizeof(int));
    file1.read(reinterpret_cast<char*>(&height1), sizeof(int));
    file2.read(reinterpret_cast<char*>(&dummy), sizeof(float));
    file2.read(reinterpret_cast<char*>(&width2), sizeof(int));
    file2.read(reinterpret_cast<char*>(&height2), sizeof(int));

    // one time flow allocations upon processing first frame
    if (!frameIndex) {
        width = width1;
        height = height1;
        numVecs = width * height;
        setup();
    }

    if (width1 != width2 || height1 != height2 || width1 != width || height1 != height) {
        cout << "FVF: mismatching forward and backward frames in frame " << frameIndex << " . Exiting." << endl;
        exit(EXIT_FAILURE);
    }

    // read forward and backward flows from files
    for (int i = 0; i != numVecs; i++) {
        file1.read(reinterpret_cast<char*>(&x), sizeof(float));
        file1.read(reinterpret_cast<char*>(&y), sizeof(float));
        xFlowFor[i] = x;
        yFlowFor[i] = y;

        file2.read(reinterpret_cast<char*>(&x), sizeof(float));
        file2.read(reinterpret_cast<char*>(&y), sizeof(float));
        xFlowBack[i] = x;
        yFlowBack[i] = y;
    }
}

/**
 * @brief Get a frame's index
 */
int FlowVectorFrame::getFrameIndex() {
    return frameIndex;
}

/**
 * @brief Get a frame's width
 */
int FlowVectorFrame::getWidth() {
    return width;
}

/**
 * @brief Get a frame's height
 */
int FlowVectorFrame::getHeight() {
    return height;
}

/**
 * @brief Get a frame's forward xFlow
 */
float* FlowVectorFrame::getXFlowFor() {
    return xFlowFor;
}

/**
 * @brief Get a frame's forward yFlow
 */
float* FlowVectorFrame::getYFlowFor() {
    return yFlowFor;
}

/**
 * @brief Get a frame's backward xFlow
 */
float* FlowVectorFrame::getXFlowBack() {
    return xFlowBack;
}

/**
 * @brief Get a frame's backward yFlow
 */
float* FlowVectorFrame::getYFlowBack() {
    return yFlowBack;
}
