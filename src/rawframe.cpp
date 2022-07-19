#include "camera-capture/rawframe.hpp"

RawFrame::RawFrame(int dtype) : height(0), width(0),  dtype(dtype)
{
    this->matrix = cv::Mat();
    info = nullptr;
}

void RawFrame::assignFrame(fbi_ptr &_info, int _width, int _height)
{
    this->height = _height;
    this->width = _width;
    this->info = _info;
}

void RawFrame::readFromFile(std::string filename, int width, int height, int mat_dtype)
{

    void* buffer;
    std::ifstream t;
    int length;
    t.open(filename, std::ios_base::in|std::ios_base::binary);

    t.seekg(0, std::ios::end);    // go to the end
    length = t.tellg();           // report location (this is the length)
    t.seekg(0, std::ios::beg);    // go back to the beginning
    buffer = malloc(length);      // allocate memory for a buffer of appropriate dimension
    //TODO: free
    t.read((char*)buffer, length);       // read the whole file into the buffer

    dtype = mat_dtype;
    height = height;
    width = width;
    matrix = cv::Mat(height, width, dtype, buffer);

    std::cout << length << " " << matrix.total() * matrix.elemSize() << " " << matrix.step[0] * matrix.rows <<std::endl;
    info = std::make_shared<FrameBufferInfo>(buffer, matrix.total() * matrix.elemSize());
}

void RawFrame::rawToCvMat()
{
    matrix  = cv::Mat(height, width, dtype, info->start);
}

cv::Mat RawFrame::getMatrix()
{
    if (matrix.empty())
    {
        rawToCvMat();
    }
    return matrix;
}

void RawFrame::saveToFile(std::string filename)
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
