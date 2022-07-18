cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(camera_capture VERSION 0.1)

find_package( OpenCV REQUIRED )

include_directories(
	${OpenCV_INCLUDE_DIRS}
	include
)

add_executable(${PROJECT_NAME}-demo
    src/example.cpp
)

add_library(cameracapture
    src/framebufferinfo.cpp
    src/cameracapture.cpp
    src/frame.cpp
    src/frames/yuvframe.cpp
    src/frames/bayerframe.cpp
)

target_link_libraries(${PROJECT_NAME}-demo PRIVATE
    cameracapture
)

target_link_libraries(${PROJECT_NAME}-demo PUBLIC
    v4l2
    ${OpenCV_LIBS}
)

add_compile_options(-Wall -Wextra -pedantic -Werror -o)

install(
    TARGETS cameracapture
    DESTINATION lib
)
