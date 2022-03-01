#ifndef VIDEO_PROCECSSOR_HPP
#define VIDEO_PROCECSSOR_HPP

#include <string>

using namespace std;

class VideoProcessor {
    private:
        string inputPath;
        int slowmoFactor;
        int videoFrameCount;
        int videoWidth;
        int videoHeight;
        void extractVideoFrames();
    public:
        VideoProcessor() {};
        VideoProcessor(string inputPath, int slowmoFactor);
        int getVideoFrameCount();
        int getVideoWidth();
        int getVideoHeight();
        // TODO
        int getFramePair(int firstIndex, int secondIndex);
};

#endif
