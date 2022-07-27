#include "camera-capture/frameconverters/raw2bayerconverter.hpp"

cv::Mat Raw2BayerConverter::convert(cv::Mat src)
{
    cv::Mat processed_frame = cv::Mat(src.rows, src.cols, destMatType);
    cv::demosaicing(src, processed_frame, code, nChannels);
    return processed_frame;
}
