#include "Frame.hpp"
#include <opencv2/imgproc.hpp> //cvtColor
#include <opencv2/imgcodecs.hpp> //imwrite
#include "opencv2/core/core_c.h"

class YuvFrame : public Frame
{
public:
    YuvFrame();
    int retreive() override;
};
