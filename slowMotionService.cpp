#include "slowMotionService.hpp"
#include "videoProcessor.hpp"

#include <zhelpers.hpp>
#include <iostream>

using namespace std;

/**
 * @brief Create a SlowMotionService::SlowMotionService object to handle all operations required for
 * interpolated frame generation
 *
 * @param input_path specifying path to input video file
 * @param slowmo_factor specifying how much the input video needs to be slowed down by
 * @param output_fps specifying fps of output video
 * @param output_path specifying path to output video file
 */
SlowMotionService::SlowMotionService(string input_path, int slowmo_factor, int output_fps, string output_path) {
    // create context
    this->context = make_unique<zmq::context_t>(1);

    // initialize requester socket on localhost:8080
    this->flow_requester = make_unique<zmq::socket_t>(*context, ZMQ_REQ);
    cout << "SMS: Connecting requester to tcp://127.0.0.1:5555..." << endl;
	flow_requester->connect("tcp://127.0.0.1:5555");

    // initialize variables
    this->input_path = input_path;
    this->slowmo_factor = slowmo_factor;
    this->output_fps = output_fps;
    this->output_path = output_path;

    // create frame reader to interface directly with video frames
    this->video_processor = make_unique<VideoProcessor>(input_path, slowmo_factor);

    // TODO: create models
}

/**
 * @brief Process video frames in pairs and created interpolated frames
 */
void SlowMotionService::startService() {
    int first_frame_index = 0;
    int last_frame_index = video_processor->getVideoFrameCount() * slowmo_factor;

    while (first_frame_index < last_frame_index) {
        // send request with frame number
        s_send(*flow_requester, to_string(first_frame_index));
        
        // receive message
        string serialized_msg = s_recv(*flow_requester);

        // deserialize message into FlowVectorFrame
        FlowVectorFrame buffer_frame;
        stringstream msg(serialized_msg);
        boost::archive::text_iarchive deserializer(msg);
        deserializer >> buffer_frame;

        // wrong object received
        if (first_frame_index != buffer_frame.getFrameIndex()) {
            cout << "SMS: Received flow vectors out of order. Retrying." << endl;
            continue;
        }

        // TODO: generate F_0_1, F_1_0, I0, and I1

        // TODO: generate intermediate frames

        first_frame_index += slowmo_factor;
    }

    // send termination request
    s_send(*flow_requester, to_string(-1));

    // TODO: reconstruct video

}
