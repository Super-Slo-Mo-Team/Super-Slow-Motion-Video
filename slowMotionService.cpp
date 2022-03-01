#include "slowMotionService.hpp"
#include "videoProcessor.hpp"
#include "config.hpp"

#include <zhelpers.hpp>
#include <iostream>

using namespace std;

// singleton class instance
SlowMotionService *SlowMotionService::slowMotionService_ = nullptr;

/**
 * @brief Create a singleton instance of FlowVectorService::FlowVectorService class
 */
SlowMotionService* SlowMotionService::GetInstance(string inputPath, int slowmoFactor, int outputFps, string outputPath) {
    if (slowMotionService_ == nullptr) {
        slowMotionService_ = new SlowMotionService(inputPath, slowmoFactor, outputFps, outputPath);
    }

    return slowMotionService_;
}

/**
 * @brief Create a SlowMotionService::SlowMotionService object to handle all operations required for
 * interpolated frame generation
 *
 * @param inputPath specifying path to input video file
 * @param slowmoFactor specifying how much the input video needs to be slowed down by
 * @param outputFps specifying fps of output video
 * @param outputPath specifying path to output video file
 */
SlowMotionService::SlowMotionService(string inputPath, int slowmoFactor, int outputFps, string outputPath) {
    // create context
    context = zmq::context_t(1);

    // initialize requester socket on localhost:8080
    flowRequester = zmq::socket_t(context, ZMQ_REQ);
    cout << "SMS: Connecting responder to " << FV_SOCKET_PATH << "..." << endl;
	flowRequester.connect(FV_SOCKET_PATH);

    // initialize variables
    this->inputPath = inputPath;
    this->slowmoFactor = slowmoFactor;
    this->outputFps = outputFps;
    this->outputPath = outputPath;

    // create frame reader to interface directly with video frames
    videoProcessor = VideoProcessor(inputPath, slowmoFactor);

    // TODO: create models
}

/**
 * @brief Process video frames in pairs and created interpolated frames
 */
void SlowMotionService::startService() {
    int firstFrameIndex = 0;
    int lastFrameIndex = videoProcessor.getVideoFrameCount() * slowmoFactor;

    while (firstFrameIndex < lastFrameIndex) {
        // TODO: remove after all .flo files are imported
        if (firstFrameIndex == 900) {
            break;
        }

        // send request with frame number
        s_send(flowRequester, to_string(firstFrameIndex));
        
        // receive message
        string serializedMsg = s_recv(flowRequester);

        // deserialize message into FlowVectorFrame
        stringstream msg(serializedMsg);
        boost::archive::binary_iarchive deserializer(msg);
        deserializer >> bufferFrame;

        // wrong object received
        if (firstFrameIndex != bufferFrame.getFrameIndex()) {
            cout << "SMS: Received flow vectors out of order. Retrying." << endl;
            continue;
        }

        // create F_0_1 and F_1_0
        // torch::Tensor xFlow_t = torch::from_blob(bufferFrame.getXFlow(), {1, 1, bufferFrame->getHeight(), bufferFrame->getWidth()});
        // torch::Tensor yFlow_t = torch::from_blob(bufferFrame.getYFlow(), {1, 1, bufferFrame->getHeight(), bufferFrame->getWidth()});

        // TODO:
        // F_0_1 = torch.cat((xFlow, yFlow), dim = 1)
        // F_1_0 = np.negative(F_0_1)

        // TODO: get frames I0 and I1

        // TODO: generate intermediate frames

        firstFrameIndex += slowmoFactor;
    }

    // send termination request
    s_send(flowRequester, to_string(-1));

    // TODO: reconstruct video

}
