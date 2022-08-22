#include "camera-capture/frameconverters/raw2bayerconverter.hpp"

#include <opencv2/imgproc.hpp> //demosaicing

cv::Mat Raw2BayerConverter::convert(cv::Mat src)
{
    cv::Mat processed_frame = cv::Mat(src.rows, src.cols, destMatType);
    cv::demosaicing(src, processed_frame, code, nChannels);
    return processed_frame;
}
