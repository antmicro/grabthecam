#include "camera-capture/frameconverters/raw2yuvconverter.hpp"

cv::Mat Raw2YuvConverter::convertMatrix(cv::Mat src)
{
    // std::cout << "YUV\n";
    cv::Mat processed_frame = cv::Mat(src.rows, src.cols, destMatType);
    cv::cvtColor(src, processed_frame, code);
    return processed_frame;
}
