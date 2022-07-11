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
    YuvFrame();

    /**
    * Convert YUV to RGB
    * @return Returns 0
    */
    int retreive() override;
};
