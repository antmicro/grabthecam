#include "camera-capture/frameconverters/raw2bayerconverter.hpp"

cv::Mat Raw2BayerConverter::convert(cv::Mat src)
{
    // std::cout << "Bayer\n";
    cv::Mat processed_frame = cv::Mat(src.rows, src.cols, destMatType);
    cv::demosaicing(src, processed_frame, code, nChannels);
    return processed_frame;
}

cv::Mat Raw2BayerConverter::convert(std::shared_ptr<MMapBuffer> src, int src_dtype, int width, int height)
{
    cv::Mat raw_frame = cv::Mat(height, width, src_dtype, src->start);
    return convert(raw_frame);
}
