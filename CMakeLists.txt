cmake_minimum_required(VERSION 3.20)
project(TaoDocker)

set(CMAKE_CXX_STANDARD 23)

find_package(CLI11 CONFIG REQUIRED)

add_executable(TaoDocker main.cpp Container.cpp Container.h)
target_link_libraries(TaoDocker PRIVATE CLI11::CLI11)
