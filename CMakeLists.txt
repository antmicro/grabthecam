cmake_minimum_required(VERSION 3.10.0)

project(camera_capture VERSION 0.0)

include_directories("${PROJECT_BINARY_DIR}")

add_library(Camera src/Camera.cpp src/Camera.hpp)

add_executable(${PROJECT_NAME} src/example.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE Camera)

target_link_libraries(${PROJECT_NAME} PUBLIC v4l2)

add_compile_options(-Wall -Wextra -pedantic -Werror)
