#include <string>
#include <vector>
#include <sstream>

using namespace std;

class FlowFrame {
    public:
    FlowFrame(istream& file, int flowFrameIndex);
    int getFlowFrameIndex();
    int getWidth();
    int getHeight();
    string getXFlow();
    string getYFlow();

    private:
    int flowFrameIndex;
    int width;
    int height;
    stringstream xFlow;
    stringstream yFlow;
};
