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

    int num_vecs = this->width * this->height;
    this->xFlow = new float[num_vecs];
    this->yFlow = new float[num_vecs];

    for (int i = 0; i != num_vecs; i++) {
        float x, y;
        file.read(reinterpret_cast<char*>(&x), sizeof(float));
        file.read(reinterpret_cast<char*>(&y), sizeof(float));

        this->xFlow[i] = x;
        this->yFlow[i] = y;
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
float* FlowVectorFrame::getXFlow() {
    return this->xFlow;
}

/**
 * @brief Get a frame's yFlow
 */
float* FlowVectorFrame::getYFlow() {
    return this->yFlow;
}
