#include "grabthecam/frameconverters/anyformat2bgrconverter.hpp"
#include <opencv2/imgproc.hpp> // cvtColor

namespace grabthecam
{

cv::Mat AnyFormat2BGRConverter::convert(cv::Mat src)
{
    cv::Mat processed_frame = cv::Mat(src.rows, src.cols, dest_mat_type);
    cv::cvtColor(src, processed_frame, code, nchannels);
    return processed_frame;
}

}; // namespace grabthecam
