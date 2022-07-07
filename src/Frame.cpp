#include "../includes/Frame.hpp"

Frame::Frame(uchar_ptr &location, ubuf_ptr &info, int width, int height)
    : height(height), width(width)
{
    this -> location = std::move(location);
    this -> info = std::move(info);
    this -> cv_mat = Mat();
}


void Frame::to_cv_mat()
{
    cv_mat = Mat(height, width, CV_8SC2, (void*)location.get());
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

int Frame::rawFrameToFile(std::string filename)
{
    // Write the data out to file
    std::ofstream outFile;
    //TODO: check if folder exists
    outFile.open(filename, std::ios::binary | std::ios::app);

    outFile.write(location.get(), (double)info.get()->bytesused);
    outFile.close();
    std::cout << "Saved as " << filename << std::endl;
    return 0;
}
