#include <string>
#include <vector>
#include <sstream>

using namespace std;

class FlowFrame {
    public:
    FlowFrame(istream& file, int vectorIndex);
    int getVectorIndex();
    int getWidth();
    int getHeight();
    string getXFlow();
    string getYFlow();

    private:
    int vectorIndex;
    int width;
    int height;
    stringstream xFlow;
    stringstream yFlow;
};
