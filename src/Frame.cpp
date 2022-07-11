#include "../includes/Frame.hpp"


Frame::Frame(int _raw_frame_dtype) : height(0), width(0)
{
    this -> raw_frame = Mat();
    this -> raw_frame_dtype = _raw_frame_dtype;
    buffer = nullptr;
}

void Frame::assignFrame(sbuf_ptr &_buffer, int _width, int _height)
{
    this -> height = _height;
    this -> width = _width;
    this -> buffer = _buffer;
}

void Frame::rawToCvMat()
{
    raw_frame = Mat(height, width, raw_frame_dtype, (void*)(buffer -> start.get()));
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
    outFile.open(filename, std::ios::binary | std::ios::app);
    if(outFile.fail())
    {
        return -1;
    }
    
    outFile.write(buffer -> start.get(), buffer -> bytesused);
    outFile.close();

    if(outFile.fail())
    {
        return -1;
    }

    return 0;
}

int Frame::processedFrameToFile(std::string filename)
{
    if (!cv::imwrite(filename, getProcessedFrame()))
    {
        return -1;
    }

    return 0;
}
