cmake_minimum_required(VERSION 3.26)
project(MatchingEngine)

set(CMAKE_CXX_STANDARD 20)

# Specify the required Boost libraries
find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(${Boost_INCLUDE_DIRS})

# Include the websocketpp directory
include_directories("/home/cheytakker/CLionProjects/websocketpp")

add_executable(MatchingEngine main.cpp)

# Link Boost libraries to your project
target_link_libraries(MatchingEngine ${Boost_LIBRARIES})

# Ensure you link against the required pthread library for threading
target_link_libraries(MatchingEngine pthread)
