#ifndef VIDEO_PROCECSSOR_HPP
#define VIDEO_PROCECSSOR_HPP

#include <string>

using namespace std;

class VideoProcessor {
    private:
        static VideoProcessor* videoProcessor_;
        string inputPath;
        int slowmoFactor;
        int videoFrameCount;
        int videoWidth;
        int videoHeight;
        void extractVideoFrames();
    public:
        VideoProcessor(string inputPath, int slowmoFactor);
        VideoProcessor(VideoProcessor &other) = delete;
        void operator=(const VideoProcessor&) = delete;
        static VideoProcessor* GetInstance(string inputPath, int slowmoFactor);
        int getVideoFrameCount();
        int getVideoWidth();
        int getVideoHeight();
        // TODO
        int getFramePair(int firstIndex, int secondIndex);
};

#endif
