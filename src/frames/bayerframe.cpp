#include "camera-capture/frames/bayerframe.hpp"

BayerFrame::BayerFrame(int code, int nChannels) : Frame(CV_8UC1), code(code), nChannels(nChannels)
{}

void BayerFrame::retreive()
{
    std::cout << "Bayer\n";
    processed_frame = cv::Mat(height, width, CV_8UC3);
    cv::demosaicing(getRawFrame(), processed_frame, code, nChannels);
}
