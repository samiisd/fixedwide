# A standalone downstream project: it must find fixedwide through find_package
# and nothing else. Building this from an install tree is the packaging test.
cmake_minimum_required(VERSION 3.25)
project(fixedwide_consumer LANGUAGES CXX)
find_package(fixedwide REQUIRED)
add_executable(consumer consumer.cpp)
target_link_libraries(consumer PRIVATE fixedwide::fixedwide)
