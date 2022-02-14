#include <string>
#include <vector>

using namespace std;

class FlowFrame {
    public:
    FlowFrame(istream& flowFile, int frameIndex);
    int getFrameIndex();
    int getWidth();
    int getHeight();
    vector<float> getXFlow();
    vector<float> getYFlow();

    private:
    int frameIndex;
    int width;
    int height;
    vector<float> xFlow;
    vector<float> yFlow;
};

class FlowVideo {
    public:
    FlowVideo(string floDir);
    vector<FlowFrame> getFlowFrames();

    private:
    vector<FlowFrame> flowFrames;
};