cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(camera_capture VERSION 0.1)

add_executable(${PROJECT_NAME}
    src/example.cpp
)

include_directories(include/camera-capture)
include_directories(include/camera-capture/frames/)

add_library(Camera
    src/Camera.cpp
)
add_library(Frame
    src/Frame.cpp
)
add_library(Frames
    src/frames/YuvFrame.cpp
    src/frames/BayerFrame.cpp
)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

target_link_libraries(${PROJECT_NAME} PRIVATE Camera)
target_link_libraries(${PROJECT_NAME} PRIVATE Frame)
target_link_libraries(${PROJECT_NAME} PRIVATE Frames)

target_link_libraries(${PROJECT_NAME} PUBLIC v4l2)
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS})

add_compile_options(-Wall -Wextra -pedantic -Werror)
