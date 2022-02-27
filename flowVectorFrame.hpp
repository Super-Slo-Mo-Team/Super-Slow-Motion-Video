#ifndef FLOW_VECTOR_FRAME_HPP
#define FLOW_VECTOR_FRAME_HPP

#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/array.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace std;

class FlowVectorFrame {
    private:
        friend class boost::serialization::access;
        int frame_index;
        int width;
        int height;
        float* xFlow;
        float* yFlow;
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & frame_index;
            ar & width;
            ar & height;
            if (Archive::is_loading::value) {
                xFlow = new float[width * height];
            }
            ar & boost::serialization::make_array<float>(xFlow, width * height);
            if (Archive::is_loading::value) {
                yFlow = new float[width * height];
            }
            ar & boost::serialization::make_array<float>(yFlow, width * height);
        }
    public:
        FlowVectorFrame() {};
        FlowVectorFrame(istream& file, int frame_index);
        int getFrameIndex();
        int getWidth();
        int getHeight();
        float* getXFlow();
        float* getYFlow();
};

#endif
