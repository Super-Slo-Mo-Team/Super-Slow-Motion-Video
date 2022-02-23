#include "config.hpp"
#include "flowVectorService.hpp"
#include "slowMotionService.hpp"

#include <thread>

using namespace std;

int main(int argc, char* argv[]) {
    // TODO: use arguments
    string input_path = "./input.y4m";
    int output_fps = 60;
    int slowmo_factor = 2;
    string output_path = "./output.mkv";

    // initialize services
    FlowVectorService fvs = FlowVectorService(FLO_PATH);
    SlowMotionService sms = SlowMotionService(input_path, slowmo_factor, output_fps, output_path);

    // create threads with appropriate entry points
    thread flow_service_thread(&FlowVectorService::startService, &fvs);
    thread slowmo_service_thread(&SlowMotionService::startService, &sms);
    
    // terminate program upon completion
    slowmo_service_thread.join();
    flow_service_thread.join();
}
