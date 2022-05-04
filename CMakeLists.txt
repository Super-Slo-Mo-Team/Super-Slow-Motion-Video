cmake_minimum_required(VERSION 3.21)
project(Super_Slow_Motion_Video)

set(CMAKE_CXX_STANDARD 14)

## configure Boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS serialization)

# micro /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/c++/v1/cmath
# line 308 changed from #include <math.h>
set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")

# configure ZeroMQ
find_package(PkgConfig)
pkg_check_modules(PC_ZeroMQ QUIET zmq)
find_path(ZeroMQ_INCLUDE_DIR NAMES zmq.hpp PATHS ${PC_ZeroMQ_INCLUDE_DIRS})
find_library(ZeroMQ_LIBRARY NAMES zmq zhelper.hpp PATHS ${PC_ZeroMQ_LIBRARY_DIRS})

# configure PyTorch
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "libtorch")
find_package(Torch REQUIRED)

# set cmake args
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

## set source files
add_executable(Super_Slow_Motion_Video service/main.cpp service/model.cpp service/flowVectorFrame.cpp 
    service/flowVectorService.cpp service/slowMotionService.cpp service/videoProcessor.cpp)

## add include directories to compile and link directives
include_directories(${Boost_INCLUDE_DIRS} ${ZeroMQ_INLCUDE_DIR})
target_link_libraries(Super_Slow_Motion_Video ${Boost_LIBRARIES} ${ZeroMQ_LIBRARY} "${TORCH_LIBRARIES}")
