# include "../includes/YuvFrame.hpp"

YuvFrame::YuvFrame() : Frame(CV_8UC2)
{}

int YuvFrame::retreive()
{
    processed_frame = cv::Mat(height, width, CV_8UC3);
    cv::cvtColor(getRawFrame(), processed_frame, cv::COLOR_YUV2BGR_YUY2);
    return 0;
}
