cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)

project(grabthecam VERSION 0.1)

find_package(OpenCV REQUIRED)
find_package(RapidJSON REQUIRED)

option(ADD_GRABTHECAM_FARSHOW_DEMO "Adds a target for grabthecam-farshow integration demo" OFF)

set(INCLUDE_DIRECTORIES
    ${OpenCV_INCLUDE_DIRS}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third-party/include/>
    $<INSTALL_INTERFACE:include>
)

include_directories(${INCLUDE_DIRECTORIES})

add_library(${PROJECT_NAME} SHARED
    src/mmapbuffer.cpp
    src/frameconverter.cpp
    src/frameconverters/yuv2bgrconverter.cpp
    src/frameconverters/packedformats2rgbconverter.cpp
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


if(ADD_GRABTHECAM_FARSHOW_DEMO)
    find_package(farshow QUIET)
    if(NOT farshow_FOUND)
        cmake_policy(SET CMP0002 OLD)
        message("farshow not found - building it")
        include(FetchContent)
        FetchContent_Declare(farshow
            GIT_REPOSITORY https://github.com/antmicro/farshow
            GIT_TAG main
        )
        FetchContent_MakeAvailable(farshow)
    endif()

    add_executable(${PROJECT_NAME}-farshow-streamer
        src/farshow-streamer-example.cpp
    )
    target_include_directories(${PROJECT_NAME}-farshow-streamer PUBLIC ${INCLUDE_DIRECTORIES})

    target_link_libraries(${PROJECT_NAME}-farshow-streamer PUBLIC
        ${PROJECT_NAME}
        v4l2
        farshow-connection
        ${OpenCV_LIBS}
    )
    install(TARGETS ${PROJECT_NAME}-farshow-streamer)
endif()

install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}-demo
    EXPORT ${PROJECT_NAME}-targets
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

configure_file(
    ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake @ONLY
)

export(
    TARGETS ${PROJECT_NAME} FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake
)
install(
    EXPORT ${PROJECT_NAME}-targets DESTINATION lib/cmake/${PROJECT_NAME}
)
install(
    DIRECTORY include/grabthecam/ DESTINATION include/grabthecam/
)
install(
    FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    DESTINATION lib/cmake/${PROJECT_NAME}
)

add_custom_target(format
    COMMAND bash -c "clang-tidy -format-style=file \$(find ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include | egrep '(*hpp|*cpp)' | tr '\\n' ' ') -p ${CMAKE_BINARY_DIR} -fix"
    COMMAND bash -c "clang-format --style=file -i \$(find ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include | egrep '(*hpp|*cpp)' | tr '\\n' ' ')"
    VERBATIM
)
