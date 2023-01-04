#include "grabthecam/frameconverters/bayer2bgrconverter.hpp"

#include <opencv2/imgproc.hpp> //demosaicing

namespace grabthecam
{

cv::Mat Bayer2BGRConverter::convert(cv::Mat src)
{
    cv::Mat processed_frame = cv::Mat(src.rows, src.cols, dest_mat_type);
    cv::demosaicing(src, processed_frame, code, nchannels);
    return processed_frame;
}

}; // namespace grabthecam
