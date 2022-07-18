#include "camera-capture/frames/yuvframe.hpp"

YuvFrame::YuvFrame(int code) : Frame(CV_8UC2), code(code)
{}

void YuvFrame::retrieve()
{
    // std::cout << "YUV\n";
    processed_frame = cv::Mat(height, width, CV_8UC3);
    cv::cvtColor(getRawFrame(), processed_frame, code);
}
