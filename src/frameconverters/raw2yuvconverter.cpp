#include "camera-capture/frameconverters/raw2yuvconverter.hpp"

cv::Mat Raw2YuvConverter::convert(cv::Mat src)
{
    cv::Mat processed_frame = cv::Mat(src.rows, src.cols, destMatType);
    cv::cvtColor(src, processed_frame, code);
    return processed_frame;
}
