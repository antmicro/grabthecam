cmake_minimum_required(VERSION 3.10.0)
project(example-build-test)
set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(grabthecam REQUIRED)

set(INCLUDE_DIRECTORIES
    ${OpenCV_INCLUDE_DIRS}
    ${grabthecam_INCLUDE_DIRS}
    ../../third-party/include
)

add_executable(example-build-test
    example.cpp
)
target_include_directories(example-build-test PUBLIC ${INCLUDE_DIRECTORIES})
target_link_libraries(example-build-test PUBLIC grabthecam)
