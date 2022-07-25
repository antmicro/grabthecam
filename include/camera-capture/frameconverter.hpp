#pragma once

#include "camera-capture/mmapbuffer.hpp"
#include <opencv2/core/mat.hpp>

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
     *
     * @param src Matrix to convert
     *
     * @return Frame in desired format
     */
    virtual cv::Mat convert(cv::Mat src) = 0;

    /**
     * Convert the frame from one format to another
     *
     * Wrap the frame in cv::Mat and perform conversion
     *
     * @param src Frame to convert
     * @param src_dtype OpenCV's datatype for source matrix (see
     * https://docs.opencv.org/3.4/d1/d1b/group__core__hal__interface.html)
     * @param width Source frame width in pixels
     * @param height source frame height in pixels
     *
     * @return Frame in desired format
     */
    virtual cv::Mat convert(std::shared_ptr<MMapBuffer> src, int src_dtype, int width, int height);
};
