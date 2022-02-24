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

/**
 * @brief Construct a new VideoProcessor::VideoProcessor object
 *
 * @param input_path specifying path to input video file
 * @param slowmo_factor specifying how much the input video needs to be slowed down by
 */
VideoProcessor::VideoProcessor(string input_path, int slowmo_factor) {
    this->input_path = input_path;
    this->slowmo_factor = slowmo_factor;
    this->video_frame_count = 0;
    this->video_width = 0;
    this->video_height = 0;

    extractVideoFrames();
}

/**
 * @brief Extract frames in yuv format from input video
 *
 */
void VideoProcessor::extractVideoFrames() {
    // open input file
    ifstream file;
    file.open(input_path, ios::binary);

    char ch = 0;
    char *ch_buffer = nullptr;
    bool found_w = false, found_h = false;
    int frame_counter = 0;

    // read file one char at a time
    while (file.get(ch)) {
        if ((ch) == 'W' && !found_w) {
            string width = "";
            file.get(ch);
            while (isdigit(ch)) {
                width += (ch);
                file.get(ch);
            }
            video_width = stoi(width);
            cout << "Video width: " << video_width << endl;
            found_w = true;
        }

        if ((ch) == 'H' && !found_h) {
            string height = "";
            file.get(ch);
            while (isdigit(ch)) {
                height += (ch);
                file.get(ch);
            }
            video_height = stoi(height);
            cout << "Video height: " << video_height << endl;
            found_h = true;
        }

        if (ch == 'F') {
            string cur = "F";
            int j = 0;
            while (file.get(ch) && j < 4) {
                cur += ch;
                j++;
            }
            
            if (cur.compare("FRAME") == 0) {
                video_frame_count++;
                int size = video_width * video_height * 3 / 2;
                
                if (ch_buffer == nullptr) {
                    ch_buffer = (char*)malloc(size);
                }
                
                // read frame
                file.read(ch_buffer, size);
                
                // create output file path
                stringstream path_builder;
                path_builder << YUV_PATH << "/_" << setfill('0') << setw(5) << frame_counter << ".yuv";
                string filename = path_builder.str();

                // output file
                ofstream output;
                output.open(filename, ios::binary);
                ostream_iterator<unsigned char> it(output);
                output.write(ch_buffer, size);
                output.close();
                
                // increment frame counter
                frame_counter += slowmo_factor;
            }
        }
    }

    file.close();
    cout << "Frames contained in video: " << video_frame_count << endl;

    if (ch_buffer != nullptr) {
        free(ch_buffer);
    }
}

/**
 * @brief Get the total number of frames in the video
 */
int VideoProcessor::getVideoFrameCount() {
    return video_frame_count;
}

/**
 * @brief Get the video width
 */
int VideoProcessor::getVideoWidth() {
    return video_width;
}

/**
 * @brief Get the video height
 */
int VideoProcessor::getVideoHeight() {
    return video_height;
}

// TODO
/**
 * @brief Get a pair of images transformed to tensors
 * 
 * @param first_frame_index denoting the pair's first frame index
 */
int getFramePair(int first_index, int second_index) {
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
