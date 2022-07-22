#include "camera-capture/frameconverters/raw2yuvconverter.hpp"

cv::Mat Raw2YuvConverter::convertMatrix(cv::Mat src)
{
    // std::cout << "YUV " << std::endl;
    cv::Mat processed_frame = cv::Mat(src.rows, src.cols, destMatType);
    cv::cvtColor(src, processed_frame, code);
    return processed_frame;
}


cv::Mat Raw2YuvConverter::convertMatrix(std::shared_ptr<MMapBuffer> src, int src_dtype, int width, int height)
{
    // std::cout << "YUV from mmapbuf\n";
    cv::Mat raw_frame = cv::Mat(height, width, src_dtype, src->start);
    return convertMatrix(raw_frame);
}
