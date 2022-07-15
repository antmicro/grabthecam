cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(camera_capture VERSION 0.1)

find_package( OpenCV REQUIRED )

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    include/camera-capture/
    include/camera-capture/frames/
)

add_executable(${PROJECT_NAME}
    src/example.cpp
)

add_library(cameracapture
    src/framebufferinfo.cpp
    src/Camera.cpp
    src/Frame.cpp
    src/frames/YuvFrame.cpp
    src/frames/BayerFrame.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    cameracapture
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    v4l2
    ${OpenCV_LIBS}
)

add_compile_options(-Wall -Wextra -pedantic -Werror -o)

install(
    TARGETS cameracapture
    DESTINATION lib
)
