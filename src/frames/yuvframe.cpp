# include "YuvFrame.hpp"

YuvFrame::YuvFrame(int code) : Frame(CV_8UC2)
{}

void YuvFrame::retreive()
{
    // std::cout << "YUV\n";
    processed_frame = cv::Mat(height, width, CV_8UC3);
    cv::cvtColor(getRawFrame(), processed_frame, cv::COLOR_YUV2BGRA_UYVY);
}
