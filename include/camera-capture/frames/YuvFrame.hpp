#include "Frame.hpp"
#include <opencv2/imgproc.hpp> //cvtColor
#include <opencv2/imgcodecs.hpp> //imwrite
#include "opencv2/core/core_c.h"

/**
* Class for processing YUY2 Frames
* For more information see Frame documentation
*/
class YuvFrame : public Frame
{
public:
    /**
     * Constructor for Yuv frames
     * @param code OpenCV's color space conversion code (see https://docs.opencv.org/4.5.2/d8/d01/group__imgproc__color__conversions.html#ga57261f12fccf872a2b2d66daf29d5bd0).
     */
    YuvFrame(int code);

    /**
    * Convert YUV to RGB
    */
    void retreive() override;
};
