cmake_minimum_required(VERSION 3.26)
project(MatchingEngine)

set(CMAKE_CXX_STANDARD 20)

# Specify the required Boost libraries
find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(${Boost_INCLUDE_DIRS})

# Include the websocketpp directory
include_directories("/home/cheytakker/CLionProjects/websocketpp")


include_directories(include)
add_library(server_lib INTERFACE
        include/worker_tasks.h)
target_include_directories(server_lib INTERFACE include)

# Find ZeroMQ library
find_library(ZMQ_LIBRARY NAMES zmq)

add_executable(MatchingEngine main.cpp
        include/websockets_data_publisher.h
        include/zmq_order_listener_updater.h
        include/worker_tasks.h)

# Link Boost libraries to your project
target_link_libraries(MatchingEngine ${Boost_LIBRARIES} server_lib)

# Link ZeroMQ library to your project
target_link_libraries(MatchingEngine ${ZMQ_LIBRARY} server_lib)

# Ensure you link against the required pthread library for threading
target_link_libraries(MatchingEngine pthread server_lib)
