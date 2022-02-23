#include "flowVectorFrame.hpp"

using namespace std;

/**
 * @brief Construct a new FlowVectorFrame::FlowVectorFrame object from a .flo file
 *
 * @param file containing unparsed flow vector information
 * @param frame_index denoting the frame
 */
FlowVectorFrame::FlowVectorFrame(istream& file, int frame_index) {
    this->frame_index = frame_index;
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
int FlowVectorFrame::getFrameIndex() {
    return this->frame_index;
}

/**
 * @brief Get a frame's width
 */
int FlowVectorFrame::getWidth() {
    return this->width;
}

/**
 * @brief Get a frame's height
 */
int FlowVectorFrame::getHeight() {
    return this->height;
}

/**
 * @brief Get a frame's xFlow
 */
string FlowVectorFrame::getXFlow() {
    return this->xFlow.str();
}

/**
 * @brief Get a frame's yFlow
 */
string FlowVectorFrame::getYFlow() {
    return this->yFlow.str();
}