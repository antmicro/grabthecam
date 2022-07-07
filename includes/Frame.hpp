#pragma once
#include "consts.hpp"
#include <opencv2/core/mat.hpp>
#include <fstream>

using cv::Mat;

class Frame
{
public:
    Frame(uchar_ptr &location, ubuf_ptr &info, int width, int heigh);
    uchar_ptr location;
    int width;
    int height;

    int rawFrameToFile(std::string filename);
    Mat getCvMat();
    int retreive(); //decode
   
private:
    ubuf_ptr info;
    Mat cv_mat;
    void to_cv_mat();    
};

using uframe_ptr = std::unique_ptr<Frame>;
