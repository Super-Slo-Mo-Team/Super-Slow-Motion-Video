#include "config.hpp"
#include "flowVectorService.hpp"
#include "slowMotionService.hpp"

#include <thread>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout << "Incorrect number of arguments. Exiting." << endl;
        exit(EXIT_FAILURE);
    }

    string input_path = argv[1];
    int output_fps = stoi(argv[2]);
    int slowmo_factor = stoi(argv[3]);
    string output_path = argv[4];

    // initialize services
    FlowVectorService fvs = FlowVectorService(FLO_PATH);
    SlowMotionService sms = SlowMotionService(input_path, slowmo_factor, output_fps, output_path);

    // create threads with appropriate entry points
    thread flow_service_thread(&FlowVectorService::startService, &fvs);
    thread slowmo_service_thread(&SlowMotionService::startService, &sms);
    
    // terminate program upon completion
    slowmo_service_thread.join();
    flow_service_thread.join();

    return EXIT_SUCCESS;
}
