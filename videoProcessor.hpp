#ifndef VIDEO_PROCECSSOR_HPP
#define VIDEO_PROCECSSOR_HPP

#include <string>

using namespace std;

class VideoProcessor {
    private:
        string input_path;
        int slowmo_factor;
        int video_frame_count;
        int video_width;
        int video_height;
        void extractVideoFrames();
    public:
        VideoProcessor(string input_path, int slowmo_factor);
        int getVideoFrameCount();
        int getVideoWidth();
        int getVideoHeight();
        // TODO
        int getFramePair(int first_index, int second_index);
};

#endif
