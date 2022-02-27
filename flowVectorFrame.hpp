#ifndef FLOW_VECTOR_FRAME_HPP
#define FLOW_VECTOR_FRAME_HPP

#include <string>
#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace std;

class FlowVectorFrame {
    private:
        friend class boost::serialization::access;
        int frame_index;
        int width;
        int height;
        vector<float> xFlow;
        vector<float> yFlow;
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & frame_index & width & height & xFlow & yFlow;
        }
    public:
        FlowVectorFrame() {};
        FlowVectorFrame(istream& file, int frame_index);
        int getFrameIndex();
        int getWidth();
        int getHeight();
        vector<float> getXFlow();
        vector<float> getYFlow();
};

#endif
