#pragma once

#include <opencv2/imgproc.hpp> //cvtColor
#include <opencv2/imgcodecs.hpp> //imwrite
#include "opencv2/core/core_c.h"

#include "Frame.hpp"


/**
* Class for processing Bayer Frames
* For more information see Frame documentation
*/
class BayerFrame : public Frame
{
public:
    /**
     * Constructor
     * 
     * @param code Color space conversion code (see https://docs.opencv.org/4.5.2/d8/d01/group__imgproc__color__conversions.html#ga57261f12fccf872a2b2d66daf29d5bd0). 
     * @param nChannels Number of channels in the destination image; if the parameter is 0, the number of the channels is derived automatically from raw matrix and code.
     */
     
    BayerFrame(int code, int nChannels=0);

    /**
    * Demosaicing
    * @return Returns 0
    */
    int retreive() override;

    int code; ///< OpenCV Color space conversion code (see: constructor)
    int nChannels; ///< number of channels in the destination image (see: constructor)
};

