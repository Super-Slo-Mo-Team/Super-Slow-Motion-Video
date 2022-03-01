#include "flowVectorFrame.hpp"

using namespace std;

/**
 * @brief Allocate float pointers for the FlowVectorFrame::FlowVectorFrame object
 */
void FlowVectorFrame::setup(int numVecs) {
    this->numVecs = numVecs;
    xFlow = new float[numVecs];
    yFlow = new float[numVecs];
}

/**
 * @brief Load a frame's info into FlowVectorFrame::FlowVectorFrame object
 *
 * @param file containing unparsed flow vector information
 * @param frameIndex denoting the frame
 */
void FlowVectorFrame::readFloFile(istream& file, int frameIndex) {
    this->frameIndex = frameIndex;
    float dummy;

    file.read(reinterpret_cast<char*>(&dummy), sizeof(float));
    file.read(reinterpret_cast<char*>(&width), sizeof(int));
    file.read(reinterpret_cast<char*>(&height), sizeof(int));

    // one time xFlow and yFlow allocation upon processing first frame
    if (!frameIndex) {
        setup(width * height);
    }

    // read xFlow and yFlow from file
    for (int i = 0; i != numVecs; i++) {
        float x, y;
        file.read(reinterpret_cast<char*>(&x), sizeof(float));
        file.read(reinterpret_cast<char*>(&y), sizeof(float));

        xFlow[i] = x;
        yFlow[i] = y;
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
 * @brief Get a frame's xFlow
 */
float* FlowVectorFrame::getXFlow() {
    return xFlow;
}

/**
 * @brief Get a frame's yFlow
 */
float* FlowVectorFrame::getYFlow() {
    return yFlow;
}
