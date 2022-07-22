#include "camera-capture/frameconverters/raw2yuvconverter.hpp"

cv::Mat Raw2YuvConverter::convert(cv::Mat src)
{
    cv::Mat processed_frame = cv::Mat(src.rows, src.cols, destMatType);
    cv::cvtColor(src, processed_frame, code);
    return processed_frame;
}

cv::Mat Raw2YuvConverter::convert(std::shared_ptr<MMapBuffer> src, int src_dtype, int width, int height)
{
    cv::Mat raw_frame = cv::Mat(height, width, src_dtype, src->start);
    return convert(raw_frame);
}
