#include "slowMotionService.hpp"
#include "videoProcessor.hpp"
#include "model.cpp"
#include "config.hpp"

#include "zhelpers.hpp"
#include <torch/script.h>
#include <iostream>
#include <stdlib.h> 

#include <boost/filesystem.hpp>

using namespace boost::filesystem;
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
    cout << "Connecting Slow Motion Service responder to " << FV_SOCKET_PATH << "..." << endl << endl;
	flowRequester.connect(FV_SOCKET_PATH);

    // initialize variables
    this->inputPath = inputPath;
    this->slowmoFactor = slowmoFactor;
    this->outputFps = outputFps;
    this->outputPath = outputPath;

    // create frame reader to interface directly with video frames
    videoProcessor = VideoProcessor::GetInstance(inputPath, slowmoFactor);

    // FOR FRONT END, CREATES 1X video to display
    stringstream out;
    out << OUT_PATH << "\\1x.yuv";
    string tempOut = out.str();

    ofstream writeOut(tempOut, ofstream::binary | std::ios::app);
    for (directory_entry& entry : directory_iterator(YUV_PATH)) {
        string filename = entry.path().string();
        char* memblock;
        streampos size;
        ifstream ifs(filename, ios::binary | ios::ate);
        if (ifs.is_open()) {
            size = ifs.tellg();
            memblock = new char[size];
            ifs.seekg(0, ios::beg);
            ifs.read(memblock, size);
            ifs.close();

            writeOut.write(memblock, size);
        }
        else {
            cout << "Could not open" << filename << endl;
        }
    }
    writeOut.close();

    stringstream ffmpegCommand;
    ffmpegCommand << "ffmpeg -hide_banner -loglevel error -f rawvideo -pix_fmt yuv420p -s:v " << videoProcessor->getVideoWidth() << "x" << videoProcessor->getVideoHeight() << " -r " << this->outputFps << " -i " << tempOut << " -c:v libx264 " << "out_1X.mp4";
    string cmd = ffmpegCommand.str();

    system(cmd.c_str());

    // 
    // UNCOMMENT to use interpolation model
    //
    // try {
    //     //stringstream command;
    //     //command << "python3 " << MODEL_SCRIPT << " --model Interpolation --checkpoint " << INTERPOLATION_CHECKPOINT_PATH;
    //     //system(command.str().c_str());
    //     interpolationModel = torch::jit::load(INTERPOLATION_MODEL_PATH);
    // } catch (const c10::Error& e) {
    //     cout << "Error loading Frame Interpolation model\n" << endl;
    //     exit(EXIT_FAILURE);
    // }
    // cout << "Interpolation Model is loaded." << endl;

    // initialize backwarp model
    try {
        stringstream command;
        command << "python3 " << MODEL_SCRIPT << " --model BackWarp --width " << videoProcessor->getVideoWidth() << " --height " << videoProcessor->getVideoHeight();
        system(command.str().c_str());
        backWarpModel = torch::jit::load(BACKWARP_MODEL_PATH);
    } catch (const c10::Error& e) {
        cerr << "Error loading BackWarp model" << endl << endl;
        exit(EXIT_FAILURE);
    }
    cout << "BackWarp Model is loaded." << endl << endl;;

    // load models to device
    if (torch::cuda::is_available()) {
        cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        cout << "Cuda is available. Running on GPU." << endl;
        cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl << endl;
        device = torch::kCUDA;
    } else {
        cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        cout << "Cuda is NOT available. Running on CPU." << endl;
        cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl << endl;
        device = torch::kCPU;
    }

    // interpolationModel.to(device);
    backWarpModel.to(device);
}

/**
 * @brief Process video frames in pairs and created interpolated frames
 */
void SlowMotionService::startService() {
    // ensure autograd is off
    torch::NoGradGuard no_grad;

    int currFrameIndex = 0;

    while (currFrameIndex < videoProcessor->getVideoFrameCount() * slowmoFactor - slowmoFactor) {
        float percent = float(float(currFrameIndex)/slowmoFactor) / float(videoProcessor->getVideoFrameCount())*100;
        cout << "Working on frame " << currFrameIndex/slowmoFactor << "/" << videoProcessor->getVideoFrameCount() << " (" << int(percent) << " %)" << endl;
        
        // send request with frame number
        s_send(flowRequester, to_string(currFrameIndex / slowmoFactor));
        
        // receive message
        string serializedMsg = s_recv(flowRequester);

        // deserialize message into FlowVectorFrame
        stringstream msg(serializedMsg);
        boost::archive::binary_iarchive deserializer(msg);
        deserializer >> bufferFrame;
      
        // wrong object received
        if (currFrameIndex / slowmoFactor != bufferFrame.getFrameIndex()) {
            cout << "SMS: Received flow vectors out of order. Retrying." << endl;
            continue;
        }

        // create F_0_1 and F_1_0
        torch::Tensor F_0_1 = torch::cat(
            {
                torch::from_blob(bufferFrame.getXFlowFor(), {1, 1, bufferFrame.getHeight(), bufferFrame.getWidth()}),
                torch::from_blob(bufferFrame.getYFlowFor(), {1, 1, bufferFrame.getHeight(), bufferFrame.getWidth()})
            },
        1);

        torch::Tensor F_1_0 = torch::cat(
            {
                torch::from_blob(bufferFrame.getXFlowBack(), {1, 1, bufferFrame.getHeight(), bufferFrame.getWidth()}),
                torch::from_blob(bufferFrame.getYFlowBack(), {1, 1, bufferFrame.getHeight(), bufferFrame.getWidth()})
            },
        1);

        F_0_1.to(device);
        F_1_0.to(device);
        
        // Get frames I0 and I1
        vector<torch::Tensor> framePair = videoProcessor->getFramePair(currFrameIndex);       
        torch::Tensor I0 = framePair[0].to(device);
        torch::Tensor I1 = framePair[1].to(device);
        
        // generate intermediate frames
        for(float i = 1; i != slowmoFactor; i++) {
            // calculate intermediate vector flows
            float t = float(i) / slowmoFactor;
            torch::Tensor F_t_0 = torch::add(((-t*(1-t)) * F_0_1), ((t*t) * F_1_0));
            torch::Tensor F_t_1 = torch::add((((1-t)*(1-t)) * F_0_1), ((-t*(1-t)) * F_1_0));
            F_t_0.to(device);
            F_t_1.to(device);
            
            // bilinearly interpolate frames at current time step t from the input frames and optical flows
            std::vector<torch::jit::IValue> backWarpInput;
            backWarpInput.push_back(I0);
            backWarpInput.push_back(F_t_0);
            torch::Tensor g_I0_F_t_0 = this->backWarpModel.forward(backWarpInput).toTensor();
            backWarpInput.clear();
            backWarpInput.push_back(I1);
            backWarpInput.push_back(F_t_1);
            torch::Tensor g_I1_F_t_1 = this->backWarpModel.forward(backWarpInput).toTensor();
            backWarpInput.clear();
            g_I0_F_t_0.to(device);
            g_I1_F_t_1.to(device); 

            F_0_1 = F_0_1.to(device);
            F_1_0 = F_1_0.to(device);

            F_t_0 = F_t_0.to(device);
            F_t_1 = F_t_1.to(device);
            
            // 
            // UNCOMMENT to use interpolation model
            //
            // // refine interpolation and generate soft visibility maps
            // torch::Tensor interpIn = torch::cat({I0, I1, F_0_1, F_1_0, F_t_1, F_t_0, g_I1_F_t_1, g_I0_F_t_0}, 1);
            // std::vector<torch::jit::IValue> interpolationInput;
            // interpolationInput.push_back(interpIn);
            // torch::Tensor interpOut = this->interpolationModel.forward(interpolationInput).toTensor();
            // torch::Tensor F_t_0_f = interpOut.slice(1, 0, 2) + F_t_0;
            // torch::Tensor F_t_1_f = interpOut.slice(1, 2, 4) + F_t_1;
            // interpolationInput.clear();

            // torch::Tensor V_t_0 = interpOut.slice(1, 4, 5);
            // torch::Tensor V_t_1 = 1 - V_t_0;
           
            // // second pass of backwarp network
            // backWarpInput.push_back(I0);
            // backWarpInput.push_back(F_t_0_f);
            // torch::Tensor g_I0_F_t_0_f = this->backWarpModel.forward(backWarpInput).toTensor();

            // backWarpInput.clear();
            // backWarpInput.push_back(I1);
            // backWarpInput.push_back(F_t_1_f);
            // torch::Tensor g_I1_F_t_1_f = this->backWarpModel.forward(backWarpInput).toTensor();
            // backWarpInput.clear();
          
            //
            // UNCOMMENT line 244 to do the interpolation step with model
            // COMMENT line 245 to do the interpolation step without the model
            //
            // // fuse warped images to create an interpolated frame
            // torch::Tensor Ft_p = ((1-t) * V_t_0 * g_I0_F_t_0_f + t * V_t_1 * g_I1_F_t_1_f ) / ((1-t) * V_t_0 + t * V_t_1);
            torch::Tensor Ft_p = (1-t) * g_I0_F_t_0 + t * g_I1_F_t_1;
            Ft_p = Ft_p.to(device);
            
            vector<char> imgFile = videoProcessor->tensorToYUV(Ft_p);

            stringstream pathBuilder;
            pathBuilder << YUV_PATH << "/" << "input_" << videoProcessor->getVideoWidth() << "x" << videoProcessor->getVideoHeight() << "_" << setfill('0') << setw(MAX_FILE_DIGITS) << currFrameIndex + int(i) << ".yuv";
            string imgName = pathBuilder.str();

            ofstream writeOut(imgName,ofstream::binary);
            writeOut.write(&imgFile[0],imgFile.size());
        }

        currFrameIndex += slowmoFactor;
    }

    // send termination request
    s_send(flowRequester, to_string(TERMINATION_MSG));

    // create output file
    stringstream out;
    out << OUT_PATH << "\\tmp.yuv";
    string tempOut = out.str();

    ofstream writeOut(tempOut, ofstream::binary | std::ios::app);
    for (directory_entry& entry : directory_iterator(YUV_PATH)) {
        string filename = entry.path().string();
        cout << "Writing: " << filename << endl;
        char* memblock;
        streampos size;
        ifstream ifs(filename, ios::binary | ios::ate);
        if (ifs.is_open()) {
            size = ifs.tellg();
            memblock = new char[size];
            ifs.seekg(0, ios::beg);
            ifs.read(memblock, size);
            ifs.close();

            writeOut.write(memblock, size);
        }
        else {
            cout << "SMS: Could not open " << filename << endl;
        }
    }
    writeOut.close();

    stringstream ffmpegCommand;
    ffmpegCommand << "ffmpeg -hide_banner -loglevel error -f rawvideo -pix_fmt yuv420p -s:v " << videoProcessor->getVideoWidth() << "x" << videoProcessor->getVideoHeight() << " -r " << this->outputFps << " -i " << tempOut << " -c:v libx264 " << this->outputPath;
    string cmd = ffmpegCommand.str();

    system(cmd.c_str());
    cout << "SMS: Video has been generated" << endl;
}
