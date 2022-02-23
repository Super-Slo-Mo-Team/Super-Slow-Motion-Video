#ifndef FLOW_VECTOR_FRAME_HPP
#define FLOW_VECTOR_FRAME_HPP

#include <string>
#include <sstream>

using namespace std;

class FlowVectorFrame {
    private:
        int frame_index;
        int width;
        int height;
        stringstream xFlow;
        stringstream yFlow;
    public:
        FlowVectorFrame(istream& file, int frame_index);
        int getFrameIndex();
        int getWidth();
        int getHeight();
        string getXFlow();
        string getYFlow();
};

#endif
