#include "camera-capture/frameconverters/raw2bayerconverter.hpp"

Raw2BayerConverter::Raw2BayerConverter(int code, int destMatType, int nChannels)
    : code(code), destMatType(destMatType), nChannels(nChannels)
{
}

cv::Mat Raw2BayerConverter::convertMatrix(cv::Mat src)
{
    // std::cout << "Bayer\n";
    cv::Mat processed_frame = cv::Mat(src.rows, src.cols, destMatType);
    cv::demosaicing(src, processed_frame, code, nChannels);
    return processed_frame;
}
