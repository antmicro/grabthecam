#pragma once

#include "camera-capture/frameconverter.hpp"

/**
 * Class for processing Bayer Frames
 * For more information see Frame documentation
 */
class Raw2BayerConverter : public FrameConverter
{
public:
    /**
     * Constructor for Bayer converter
     *
     * @param code OpenCV's color space conversion code (see
     * https://docs.opencv.org/4.5.2/d8/d01/group__imgproc__color__conversions.html#ga57261f12fccf872a2b2d66daf29d5bd0).
     * @param destMatType OpenCV's datatype for destination matrix (see
     * https://docs.opencv.org/3.4/d1/d1b/group__core__hal__interface.html)
     * @param nChannels Number of channels in the destination image; if the parameter is 0, the number of the channels
     * is derived automatically from raw matrix and code.
     */
    Raw2BayerConverter(int code, int destMatType = CV_8UC3, int nChannels = 0)
        : code(code), destMatType(destMatType), nChannels(nChannels)
    {
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
    int destMatType; ///< OpenCV's datatype for destination matrix (see: constructor)
    int nChannels;   ///< number of channels in the destination image (see: constructor)
};
