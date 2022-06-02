#ifndef FLOW_VECTOR_FRAME_HPP
#define FLOW_VECTOR_FRAME_HPP

#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/array.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace std;

class FlowVectorFrame {
    private:
        friend class boost::serialization::access;
        int frameIndex;
        int width;
        int height;
        int numVecs;
        float* xFlowFor;
        float* yFlowFor;
        float* xFlowBack;
        float* yFlowBack;
        void setup();
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & frameIndex;
            ar & width;
            ar & height;
            if (Archive::is_loading::value) {
                xFlowFor = new float[width * height];
            }
            ar & boost::serialization::make_array<float>(xFlowFor, width * height);
            if (Archive::is_loading::value) {
                yFlowFor = new float[width * height];
            }
            ar & boost::serialization::make_array<float>(yFlowFor, width * height);
            if (Archive::is_loading::value) {
                xFlowBack = new float[width * height];
            }
            ar & boost::serialization::make_array<float>(xFlowBack, width * height);
            if (Archive::is_loading::value) {
                yFlowBack = new float[width * height];
            }
            ar & boost::serialization::make_array<float>(yFlowBack, width * height);
        };
    public:
        FlowVectorFrame() {};
        ~FlowVectorFrame() {
            free(xFlowFor);
            free(yFlowFor);
            free(xFlowBack);
            free(yFlowBack);
        };
        void readFloFile(istream& file1, istream& file2, int frameIndex);
        int getFrameIndex();
        int getWidth();
        int getHeight();
        float* getXFlowFor();
        float* getYFlowFor();
        float* getXFlowBack();
        float* getYFlowBack();
};

#endif
