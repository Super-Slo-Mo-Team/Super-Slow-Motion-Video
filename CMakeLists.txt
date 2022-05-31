cmake_minimum_required(VERSION 3.21)
project(Super_Slow_Motion_Video)

set(CMAKE_CXX_STANDARD 14)

#HERE 

find_package(BOOST REQUIRED COMPONENTS serialization)
set(BOOST_LIBRARIES ${BOOST_LIBRARIES} "C:\\Program Files\\boost\\boost_1_79_0\\stage\\lib\\libboost_serialization-vc143-mt-s-x64-1_79.lib")
set(BOOST_INCLUDE ${BOOST_INCLUDE} "C:\\Program Files\\boost\\boost_1_79_0")


message(STATUS "Boost version: ${Boost_VERSION}")  
message(${BOOST_INCLUDE})
message(${BOOST_LIBRARIES})
message(${CMAKE_CXX_COMPILER_ID}) 

add_library(BOOST_LIBRARY STATIC IMPORTED) # or STATIC instead of SHARED 
set_target_properties(BOOST_LIBRARY PROPERTIES
  IMPORTED_LOCATION "${BOOST_LIBRARIES}"
  INTERFACE_INCLUDE_DIRECTORIES "${BOOST_INCLUDE}"
)


 
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:\\Repos\\vcpkg\\packages\\zeromq_x64-windows-static")
find_package(ZeroMQ CONFIG REQUIRED)
set(ZeroMQ_INCLUDE_DIR "${CMAKE_PREFIX_PATH}\\include")
include_directories(${ZeroMQ_INCLUDE_DIR})
set(ZeroMQ_LIBRARIES ${CMAKE_PREFIX_PATH}\\lib\\libzmq-mt-s-4_3_4.lib)
message(${ZeroMQ_LIBRARIES}) 
message(${ZeroMQ_INCLUDE_DIR})

 
 
add_library(ZMQ_LIBRARY STATIC IMPORTED) # or STATIC instead of SHARED
set_target_properties(ZMQ_LIBRARY PROPERTIES
  IMPORTED_LOCATION "${ZeroMQ_LIBRARIES}"
  INTERFACE_INCLUDE_DIRECTORIES "${ZeroMQ_INCLUDE_DIR}"
)

set(CUDNN_LIBRARY_PATH ${CUDNN_LIBRARY_PATH} "C:\\Program Files\\NVIDIA GPU Computing Toolkit\\CUDA\\v11.6\\bin")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH}  "C:\\Program Files\\libtorchRelease\\libtorch\\")
find_package(Torch REQUIRED)



# set cmake args
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
message(${CMAKE_CXX_FLAGS})

add_executable(Super_Slow_Motion_Video service/main.cpp service/model.cpp service/flowVectorFrame.cpp 
    service/flowVectorService.cpp service/slowMotionService.cpp service/videoProcessor.cpp)

set_property(TARGET Super_Slow_Motion_Video PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

## add include directories to compile and link directives
include_directories(${BOOST_INCLUDE} ${ZeroMQ_INCLUDE_DIR})
#
target_link_libraries(Super_Slow_Motion_Video BOOST_LIBRARY ZMQ_LIBRARY ${TORCH_LIBRARIES} wsock32 ws2_32 Iphlpapi)