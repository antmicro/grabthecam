cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(camera_capture VERSION 0.1)

add_executable(${PROJECT_NAME}
  src/example.cpp
)

include_directories(include)

add_library(Camera
  src/Camera.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE Camera)
target_link_libraries(${PROJECT_NAME} PUBLIC v4l2)

add_compile_options(-Wall -Wextra -pedantic -Werror)
