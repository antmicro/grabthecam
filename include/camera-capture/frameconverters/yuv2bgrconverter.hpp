#pragma once

#include "camera-capture/frameconverter.hpp"

/**
 * Class for processing YUV Frames
 * For more information see FrameConverter documentation
 */
class Yuv2BGRConverter : public FrameConverter
{
public:
    Yuv2BGRConverter() {}

    /**
     * Constructor for Yuv converter
     *
     * @param code OpenCV's color space conversion code (see
     * https://docs.opencv.org/4.5.2/d8/d01/group__imgproc__color__conversions.html#ga57261f12fccf872a2b2d66daf29d5bd0).
     * @param inputFormat_ OpenCV's datatype for input (raw) matrix
     * @param dest_mat_type OpenCV's datatype for destination matrix (see
     * https://docs.opencv.org/3.4/d1/d1b/group__core__hal__interface.html)
     */
    Yuv2BGRConverter(int code, int input_format = CV_8UC2, int dest_mat_type = CV_8UC3)
        : code(code), dest_mat_type(dest_mat_type)
    {
        this->input_format = input_format;
    }

    /**
     * Convert YUV to RGB
     *
     * @param src Matrix to convert
     * @return Converted frame
     */
    cv::Mat convert(cv::Mat src) override;

private:
    int code;        ///< OpenCV's Color space conversion code (see: constructor)
    int dest_mat_type; ///< OpenCV's datatype for destination matrix (see: constructor)
};
