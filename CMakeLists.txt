cmake_minimum_required(VERSION 3.20)
project(TaoDocker)

set(CMAKE_CXX_STANDARD 23)

find_package(CLI11 CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)

add_executable(TaoDocker main.cpp Container.cpp Container.h Cgroup.cpp Cgroup.h)
target_link_libraries(TaoDocker PRIVATE CLI11::CLI11 absl::strings absl::str_format)
