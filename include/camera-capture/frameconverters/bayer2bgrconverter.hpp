#pragma once

#include "camera-capture/frameconverter.hpp"

/**
 * Class for processing Bayer Frames
 * For more information see Frame documentation
 */
class Bayer2BGRConverter : public FrameConverter
{
public:
    /**
     * Constructor for Bayer converter
     *
     * @param code OpenCV's color space conversion code (see
     * https://docs.opencv.org/4.5.2/d8/d01/group__imgproc__color__conversions.html#ga57261f12fccf872a2b2d66daf29d5bd0).
     * @param input_format OpenCV's datatype for input (raw) matrix
     * @param dest_mat_type OpenCV's datatype for destination matrix (see
     * https://docs.opencv.org/3.4/d1/d1b/group__core__hal__interface.html)
     * @param nchannels Number of channels in the destination image; if the parameter is 0, the number of the channels
     * is derived automatically from raw matrix and code.
     */
    Bayer2BGRConverter(int code, int input_format = CV_8UC1, int dest_mat_type = CV_8UC3, int nchannels = 0)
        : code(code), dest_mat_type(dest_mat_type), nchannels(nchannels)
    {
        this->input_format = input_format;
    }

    /**
     * Perform demosaicing
     *
     * @param src Matrix to convert
     * @return Converted frame
     */
    cv::Mat convert(cv::Mat src) override;

private:
    int code;        ///< OpenCV's Color space conversion code (see: constructor)
    int dest_mat_type; ///< OpenCV's datatype for destination matrix (see: constructor)
    int nchannels;   ///< number of channels in the destination image (see: constructor)
};
