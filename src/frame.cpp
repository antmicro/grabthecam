#include "camera-capture/frame.hpp"

cv::Mat Frame::getMatrix() { return matrix; }

void Frame::saveToFile(std::string filename)
{
    // check if directory exists
    std::filesystem::path path = filename;
    std::filesystem::create_directories(path.parent_path());

    if (!cv::imwrite(filename, matrix))
    {
        throw CameraException("Cannot save the processed Frame");
    }
}
