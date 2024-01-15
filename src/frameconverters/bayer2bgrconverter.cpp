#include "grabthecam/frameconverters/bayer2bgrconverter.hpp"

#include <opencv2/imgproc.hpp> //demosaicing

namespace grabthecam
{

cv::Mat Bayer2BGRConverter::convert(cv::Mat src)
{
    cv::Mat processed_frame = cv::Mat(src.rows, src.cols, dest_mat_type);
    if (this->input_format == CV_16UC1)
    {
        src.convertTo(src, CV_8UC1);
        cv::demosaicing(src, processed_frame, code, nchannels);
    }
    else
    {
        cv::demosaicing(src, processed_frame, code, nchannels);
    }
    return processed_frame;
}

}; // namespace grabthecam
