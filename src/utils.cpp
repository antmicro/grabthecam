#include "grabthecam/utils.hpp"

#include <filesystem> // checking if the directory exists
#include <fstream>    // ofstream
#include <iostream>
#include <opencv2/imgcodecs.hpp> // imwrite

namespace grabthecam
{

void createDirectories(std::string filename)
{
    std::filesystem::path path = filename;
    std::filesystem::path parent = path.parent_path();
    if (parent != "")
    {
        std::filesystem::create_directories(parent);
    }
}

void rawToFile(std::string filename, std::shared_ptr<MMapBuffer> frame)
{
    std::cout << "Saving " << filename << std::endl;

    createDirectories(filename);

    // Write the data out to file
    std::ofstream out_file;
    out_file.open(filename, std::ios::binary);
    if (out_file.fail())
    {
        throw CameraException("Cannot open the file to save. Check if file exists and you have permission to edit it.");
    }

    out_file.write((char *)(frame->start), frame->bytesused);
    out_file.close();
}

void saveToFile(std::string filename, cv::Mat &frame)
{
    std::cout << "Saving " << filename << std::endl;

    createDirectories(filename);
    if (!cv::imwrite(filename, frame))
    {
        throw CameraException("Cannot save the processed Frame");
    }
}

};
