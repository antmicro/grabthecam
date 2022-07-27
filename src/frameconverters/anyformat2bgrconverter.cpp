#include "camera-capture/frameconverters/anyformat2bgrconverter.hpp"

cv::Mat AnyFormat2bgrConverter::convert(cv::Mat src)
{
    cv::Mat processed_frame = cv::Mat(src.rows, src.cols, destMatType);
    cv::cvtColor(src, processed_frame, code, nChannels);
    return processed_frame;
}
