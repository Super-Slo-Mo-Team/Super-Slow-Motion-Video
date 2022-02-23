#include "frameLoader.hpp"

#include <iostream>

/**
 * @brief Construct a new FrameLoader::FrameLoader object
 *
 * @param input_path specifying path to input video file
 * @param slowmo_factor specifying how much the input video needs to be slowed down by
 */
FrameLoader::FrameLoader(string input_path, int slowmo_factor) {
    this->input_path = input_path;
    this->slowmo_factor = slowmo_factor;
    this->frame_count = 0;
    this->frame_width = 0;
    this->frame_height = 0;

    extractVideoFrames();
}

/**
 * @brief Extract individual frames from video
 */
void FrameLoader::extractVideoFrames() {
    // extract frames from video
    // string ffmpeg_command = sprintf("ffmpeg -i %s -vsync 0 %s/_%05d.png", input_path.c_str(), R_FRAME_PATH.c_str());
    int retval = system("ls -l");
    if (retval) {
        cout << "Error converting video file." << endl;
        exit(EXIT_FAILURE);
    }
    
    // extract frame count
    // self.frameCount = len(fnmatch.filter(os.listdir(dirPaths["rFrames"]), '*.png'))

    // // set frame width and height by opening first file
    // with open(dirPaths["rFrames"] + '/' + FIRST_RAW_FRAME_FILENAME, 'rb') as file:
    //     firstFrame = Image.open(file).convert('RGB')
    //     self.frameWidth, self.frameHeight = firstFrame.size

    // // rename frame files to give space for intermediate frames
    // index = 0
    // for _, filename in enumerate(sorted(os.listdir(dirPaths["rFrames"]))):
    //     if filename.endswith('.png'):
    //         os.rename(dirPaths["rFrames"] + '/' + filename, dirPaths["iFrames"] + '/_' + str(index).zfill(5) + '.png')
    //         index += self.slowdown
}

/**
 * @brief Get the total number of frames
 */
int FrameLoader::getFrameCount() {
    return frame_count;
}

/**
 * @brief Get the frame width
 */
int FrameLoader::getWidth() {
    return frame_width;
}

/**
 * @brief Get the frame height
 */
int FrameLoader::getHeight() {
    return frame_height;
}

/**
 * @brief Get a pair of images transformed to tensors
 * 
 * @param first_frame_index denoting the pair's first frame index
 */
int getFramesByFirstIndex(int first_frame_index) {
    // I0, I1 = None, None

    // with open(dirPaths["iFrames"] + '/_%05d.png' % firstFrameIndex, 'rb') as file:
    //     I0 = Image.open(file).convert('RGB')
    // with open(dirPaths["iFrames"] + '/_%05d.png' % (firstFrameIndex + self.slowdown), 'rb') as file:
    //     I1 = Image.open(file).convert('RGB')

    // I0 = transforms.ToTensor()(I0).unsqueeze_(0)
    // I1 = transforms.ToTensor()(I1).unsqueeze_(0)

    // return I0, I1
}
