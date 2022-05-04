#ifndef VIDEO_PROCECSSOR_HPP
#define VIDEO_PROCECSSOR_HPP

#include <string>
#include <torch/torch.h>
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
        
        vector<torch::Tensor> getFramePair(int frameIndex);
        static vector<int> ReadAllBytes(string filename);
        torch::Tensor fileToTensor(string file);
        void mapColor(std::vector<int> values,torch::Tensor* colorT);
        void yTensor(std::vector<int> values, torch::Tensor* yT);

        vector<char> tensorToYUV(torch::Tensor img);
        vector<int> resizeColorVector(torch::Tensor color);
};

#endif
