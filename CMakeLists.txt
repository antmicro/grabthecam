cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)

project(grabthecam VERSION 0.1)

find_package(OpenCV REQUIRED)
find_package(RapidJSON REQUIRED)

set(INCLUDE_DIRECTORIES
    ${OpenCV_INCLUDE_DIRS}
    include
    third-party/include/
    third-party/rapidjson/include/
)

add_library(${PROJECT_NAME} SHARED
    src/mmapbuffer.cpp
    src/frameconverter.cpp
    src/frameconverters/yuv2bgrconverter.cpp
    src/frameconverters/bayer2bgrconverter.cpp
    src/frameconverters/anyformat2bgrconverter.cpp
    src/utils.cpp
    src/cameracapture.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRECTORIES})
target_link_libraries(${PROJECT_NAME} PUBLIC
    v4l2
    ${OpenCV_LIBS}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
)

add_executable(${PROJECT_NAME}-demo
    src/example.cpp
)

target_include_directories(${PROJECT_NAME}-demo PUBLIC ${INCLUDE_DIRECTORIES})

target_link_libraries(${PROJECT_NAME}-demo PRIVATE
    ${PROJECT_NAME}
)

target_link_libraries(${PROJECT_NAME}-demo PUBLIC
    v4l2
    ${OpenCV_LIBS}
)

install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}-demo
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(
    DIRECTORY include/grabthecam/ DESTINATION include/grabthecam/
)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_custom_target(format
    COMMAND bash -c "find ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include -iname \"*.cpp\" -o -iname \"*.hpp\" |xargs clang-tidy -format-style=file -p ${CMAKE_BINARY_DIR} -fix"
    COMMAND bash -c "find ${CMAKE_SOURCE_DIR} ${CMAKE_INCLUDE_DIR} -iname \"*.cpp\" -o -iname \"*.hpp\"| xargs clang-format --style=file -i"
    VERBATIM
)
