
#include <string>
#include <vector>


using namespace std;

class Pixel{
    public:
    Pixel(float x, float y);
    float x();
    float y();

    private:
    float xVec;
    float yVec;

};
class FlowFrame{
    public:
    FlowFrame(istream& flowFile,int frameI);
    
    Pixel getPixelAt(int x, int y);
    int getFrameIndex();

    private:
    int frameNumber;
    int width;
    int height;

    vector<vector<Pixel> > flowVectors;

};

class FlowVideo{
    public:
    FlowVideo(string outPath);
    vector<FlowFrame> getFlowFrames();

    private:
    vector<FlowFrame> flowFrames;
    int frameIndx;

};

