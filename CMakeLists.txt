cmake_minimum_required(VERSION 3.21)
project(Super_Slow_Motion_Video)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

## configure Boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS serialization)

find_package(PkgConfig)
pkg_check_modules(PC_ZeroMQ QUIET zmq)
find_path(ZeroMQ_INCLUDE_DIR NAMES zmq.hpp PATHS ${PC_ZeroMQ_INCLUDE_DIRS})

## use the hint from about to find the location of libzmq
find_library(ZeroMQ_LIBRARY NAMES zmq zhelper.hpp PATHS ${PC_ZeroMQ_LIBRARY_DIRS})

## set source files
add_executable(Super_Slow_Motion_Video main.cpp flowVectorFrame.cpp flowVectorService.cpp slowMotionService.cpp videoProcessor.cpp)

## add include directories to compile and link directives
include_directories(${Boost_INCLUDE_DIRS} ${ZeroMQ_INLCUDE_DIR})
target_link_libraries(Super_Slow_Motion_Video ${Boost_LIBRARIES} ${ZeroMQ_LIBRARY})
