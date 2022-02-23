#ifndef FRAME_LOADER_HPP
#define FRAME_LOADER_HPP

#include <string>

using namespace std;

class FrameLoader {
    private:
        string input_path;
        int slowmo_factor;
        int frame_count;
        int frame_width;
        int frame_height;
        void extractVideoFrames();
    public:
        FrameLoader(string input_path, int slowmo_factor);
        int getFrameCount();
        int getWidth();
        int getHeight();
        // TODO
        int getFramesByFirstIndex(int first_frame_index);
};

#endif
