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
    // check if directory exists
    std::filesystem::path path = filename;
    std::filesystem::create_directories(path.parent_path());

    // Write the data out to file
    std::ofstream outFile;
    outFile.open(filename, std::ios::binary);
    if(outFile.fail())
    {
        throw CameraException("Cannot open the file to save. Check if file exists and you have permission to edit it.");
        return -1;
    }

    outFile.write((char*)(info->start), info->bytesused);
    outFile.close();

    return 0;
}

int Frame::processedFrameToFile(std::string filename)
{
    // check if directory exists
    std::filesystem::path path = filename;
    std::filesystem::create_directories(path.parent_path());

    if (!cv::imwrite(filename, getProcessedFrame()))
    {
        throw CameraException("Cannot save the processed Frame");
        return -1;
    }

    return 0;
}
