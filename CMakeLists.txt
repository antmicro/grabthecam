cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(camera_capture VERSION 0.1)

find_package( OpenCV REQUIRED )

include_directories(
	${OpenCV_INCLUDE_DIRS}
	include
)

add_executable(${PROJECT_NAME}
    src/example.cpp
)

add_library(framebufferinfo
    src/framebufferinfo.cpp
)
add_library(camera
    src/camera.cpp
)
add_library(frame
    src/frame.cpp
)
add_library(frames
    src/frames/yuvframe.cpp
    src/frames/bayerframe.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	camera
	frame
	frames
	framebufferinfo
)

target_link_libraries(${PROJECT_NAME} PUBLIC
	v4l2
	${OpenCV_LIBS}
)

add_compile_options(-Wall -Wextra -pedantic -Werror)
