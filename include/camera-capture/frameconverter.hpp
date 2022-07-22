#pragma once

#include <opencv2/core/mat.hpp>
#include "camera-capture/mmapbuffer.hpp"


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
     * Convert the matrix with frame from one format to another
     */
    virtual cv::Mat convertMatrix(cv::Mat src) = 0;
    virtual cv::Mat convertMatrix(MMapBuffer src, int src_dtype, int width, int height) = 0;
};
