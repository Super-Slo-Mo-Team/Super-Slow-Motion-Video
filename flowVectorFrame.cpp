#include "flowVectorFrame.hpp"

using namespace std;

/**
 * @brief Construct a new FlowVectorFrame::FlowVectorFrame object from a .flo file
 *
 * @param file containing unparsed flow vector information
 * @param frameIndex denoting the frame
 */
FlowVectorFrame::FlowVectorFrame(istream& file, int frameIndex) {
    this->frameIndex = frameIndex;
    float dummy;

    file.read(reinterpret_cast<char*>(&dummy), sizeof(float));
    file.read(reinterpret_cast<char*>(&width), sizeof(int));
    file.read(reinterpret_cast<char*>(&height), sizeof(int));

    int numVecs = width * height;
    xFlow = new float[numVecs];
    yFlow = new float[numVecs];

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
