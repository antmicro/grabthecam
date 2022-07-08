#include "../includes/Frame.hpp"


Frame::Frame(int _raw_frame_dtype) : height(0), width(0)
{
    this -> location = nullptr;
    this -> info = nullptr;
    this -> raw_frame = Mat();
    this -> raw_frame_dtype = _raw_frame_dtype;
}

void Frame::assignFrame(uchar_ptr &_location, ubuf_ptr &_info, int _width, int _height)
{
    this -> height = _height;
    this -> width = _width;
    this -> location = std::move(_location);
    this -> info = std::move(_info);
}

void Frame::rawToCvMat()
{
    raw_frame = Mat(height, width, raw_frame_dtype, (void*)location.get());
}

cv::Mat Frame::getRawFrame()
{

    if (raw_frame.empty())
    {
        rawToCvMat();
    }
    return raw_frame;
}

Mat Frame::getProcessedFrame()
{
    if (processed_frame.empty())
    {
        retreive();
    }
    return processed_frame;
}

int Frame::rawFrameToFile(std::string filename)
{
    // Write the data out to file
    std::ofstream outFile;
    //TODO: check if folder exists
    outFile.open(filename, std::ios::binary | std::ios::app);

    outFile.write(location.get(), (double)info.get()->bytesused);
    outFile.close();
    std::cout << "Raw frame saved as " << filename << std::endl;
    return 0;
}

int Frame::processedFrameToFile(std::string filename)
{
    cv::imwrite(filename, getProcessedFrame());
    std::cout << "Processed frame saved as " << filename << std::endl;
}
