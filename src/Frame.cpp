#include "Frame.hpp"


Frame::Frame(int _raw_frame_dtype) : height(0), width(0)
{
    this->raw_frame = cv::Mat();
    this->raw_frame_dtype = _raw_frame_dtype;
    info = nullptr;
}

void Frame::assignFrame(fbi_ptr &_info, int _width, int _height)
{
    this->height = _height;
    this->width = _width;
    this->info = _info;
}

void Frame::rawToCvMat()
{
    raw_frame = cv::Mat(height, width, raw_frame_dtype, info->start);
}

cv::Mat Frame::getRawFrame()
{

    if (raw_frame.empty())
    {
        rawToCvMat();
    }
    return raw_frame;
}

cv::Mat Frame::getProcessedFrame()
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
    outFile.open(filename, std::ios::binary);
    if(outFile.fail())
    {
        return -1;
    }

    outFile.write((char*)(info->start), info->bytesused);
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
