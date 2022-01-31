
#include <string>
#include <vector>


using namespace std;

class FlowFrame{
    public:
    FlowFrame(istream& flowFile,int frameI);
    vector<float> getXFlow();
    vector<float> getYFlow();
    int getFrameIndex();

    private:
    int frameNumber;
    int width;
    int height;
    vector<float> xFlow;
    vector<float> yFlow;


};

class FlowVideo{
    public:
    FlowVideo(string outPath);
    vector<FlowFrame> getFlowFrames();

    private:
    vector<FlowFrame> flowFrames;
    int frameIndx;

};

