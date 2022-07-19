#include "camera-capture/frameconverter.hpp"

Frame FrameConverter::convert(Frame* src)
{
    cv::Mat dest = convertMatrix(src->getMatrix());
    return Frame(dest);
}
