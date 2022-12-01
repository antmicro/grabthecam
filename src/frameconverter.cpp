#include "grabthecam/frameconverter.hpp"

namespace grabthecam
{

cv::Mat FrameConverter::convert(std::shared_ptr<MMapBuffer> src, int src_dtype, int width, int height)
{
    cv::Mat raw_frame = cv::Mat(height, width, src_dtype, src->start);
    return convert(raw_frame);
}

};
