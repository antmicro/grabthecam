#include "../includes/Frame.hpp"

Frame::Frame(uchar_ptr location, ubuf_ptr buffer_info, int width, int heigh)
    : heigh(heigh), width(width)
{
    this -> location = std::move(location);
    this -> buffer_info = std::move(buffer_info);
    this -> cv_mat = Mat();
}


void Frame::to_cv_mat()
{
    cv_mat = Mat(heigh, width, CV_8SC2, (void*)location.get()); 
    // check data type https://docs.opencv.org/4.x/d3/d63/classcv_1_1Mat.html#a51615ebf17a64c968df0bf49b4de6a3a
}

cv::Mat Frame::getCvMat()
{
    if (cv_mat.empty())
    {
        to_cv_mat();
    }
    return cv_mat;
}

int retreive()
{
    std::cout << "Preprocessing...\n";
    //TODO: abstract method implemented for specific types of frames
    return 0;
}
