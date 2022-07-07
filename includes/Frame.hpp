#pragma once
#include "consts.hpp"
#include <opencv2/core/mat.hpp>
#include <fstream>

using cv::Mat;

class Frame
{
public:
    /**
    * Frame constructor
    * @param location Begin of memory, where the frame is stored
    * @param info Contains data exchanged by application and driver using one of the Streaming I/O methods.
    * @param width Image width in pixels
    * @param height Image height in pixels
    */
    Frame(uchar_ptr &location, ubuf_ptr &info, int width, int heigh);
    uchar_ptr location;
    int width;
    int height;

    /**
    * Save raw frame
    * @param filename Filename, where the frame will be saved
    */
    int rawFrameToFile(std::string filename);

    /*
    * Get frame as openCV matrix
    * @return Returns Frame
    */
    Mat getCvMat();

    /*
    * Preprocess the raw frame
    * <THIS METHOD WILL BE ABSTRACT>
    */
    int retreive(); //decode

private:
    ubuf_ptr info;
    Mat cv_mat;

    /*
    * Convert the frame to an openCV matrix
    */
    void to_cv_mat();
};

using uframe_ptr = std::unique_ptr<Frame>;
