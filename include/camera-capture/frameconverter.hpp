#pragma once
#include "camera-capture/frame.hpp"

/**
 * Converts the frame form one format to another.
 *
 * Provides C++ API for processing frames
 * See how it can be used in src/example.cpp.
 */

class FrameConverter
{
public:
    /**
     * Convert the Frame from one format to another
     */
    virtual Frame convert(Frame *src);

    /**
     * Convert the matrix with frame from one format to another
     */
    virtual cv::Mat convertMatrix(cv::Mat src) = 0;
};
