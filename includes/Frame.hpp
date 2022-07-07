#pragma once
#include "consts.hpp"
#include <opencv2/core/mat.hpp>

using cv::Mat;

class Frame
{
public:
    Frame(uchar_ptr location, ubuf_ptr buffer_info, int width, int heigh);
    uchar_ptr location;
    int width;
    int heigh;
  
    Mat getCvMat();
    int retreive(); //decode
private:
    ubuf_ptr buffer_info;
    Mat cv_mat;
    void to_cv_mat();    
};
