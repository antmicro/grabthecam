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

void Frame::readFromFile(std::string filename, int width, int height)
{

    char* buffer;
    std::ifstream t;
    int length;
    t.open(filename);

    t.seekg(0, std::ios::end);    // go to the end
    length = t.tellg();           // report location (this is the length)
    t.seekg(0, std::ios::beg);    // go back to the beginning
    buffer = new char[length];    // allocate memory for a buffer of appropriate dimension
    t.read(buffer, length);       // read the whole file into the buffer

    raw_frame = cv::Mat(height, width, CV_8UC1, buffer);
    height = height;
    width = width;
    std::cout << length << " " << raw_frame.total() * raw_frame.elemSize() << " " << raw_frame.step[0] * raw_frame.rows <<std::endl;
    info = std::make_shared<FrameBufferInfo>(buffer, raw_frame.total() * raw_frame.elemSize());

    std::cout << raw_frame.total() << std::endl;
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

void Frame::rawFrameToFile(std::string filename)
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
    }

    outFile.write((char*)(info->start), info->bytesused);
    outFile.close();
}

void Frame::processedFrameToFile(std::string filename)
{
    // check if directory exists
    std::filesystem::path path = filename;
    std::filesystem::create_directories(path.parent_path());

    if (!cv::imwrite(filename, getProcessedFrame()))
    {
        throw CameraException("Cannot save the processed Frame");
    }
}
