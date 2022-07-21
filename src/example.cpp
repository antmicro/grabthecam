#include <sstream>

#include "camera-capture/cameracapture.hpp"
#include <filesystem> // checking if the directory exists
#include <opencv2/imgcodecs.hpp> //imwrite

void rawToFile(std::string filename, std::shared_ptr<MMapBuffer> info)
{
    std::cout << "Save\n";
    // check if directory exists
    std:: filesystem::path path = filename;
    std::filesystem::create_directories(path.parent_path());

    // Write the data out to file
    std::ofstream out_file;
    out_file.open(filename, std::ios::binary);
    if (out_file.fail())
    {
        throw CameraException("Cannot open the file to save. Check if file exists and you have permission to edit it.");
    }

    out_file.write((char *)(info->start), info->bytesused);
    out_file.close();
}

void saveToFile(std::string filename, std::shared_ptr<cv::Mat> frame)
{
    std::cout << "Save\n";
    // check if directory exists
    std::filesystem::path path = filename;
    std::filesystem::create_directories(path.parent_path());

    if (!cv::imwrite(filename, *frame.get()))
    {
        throw CameraException("Cannot save the processed Frame");
    }
}

int main(int argc, char const *argv[])
{
    std::cout << "READ RAW IMAGE FROM FILE\n--------------------------\n";

    std::cout << "\nSET CAMERA\n--------------------------\n";
    // get camera capabilities
    auto cap = std::make_unique<v4l2_capability>();

    CameraCapture camera("/dev/video0");
    camera.getCapabilities(cap);

    if (!(cap->capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        std::cerr << "This is not a video capture device\n";
        exit(EXIT_FAILURE);
    }

    if (!(cap->capabilities & V4L2_CAP_STREAMING))
    {
        std::cerr << "V4L2_CAP_STREAMING failed\n";
        exit(EXIT_FAILURE);
    }

    // adjust camera settings
    camera.setFormat(960, 720, V4L2_PIX_FMT_YYUV);
    camera.set(V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_MANUAL);

    double val;
    camera.get(V4L2_CID_EXPOSURE_AUTO, val);
    std::cout << "Value of V4L2_CID_EXPOSURE_AUTO: " << val << std::endl;

    std::cout << "\nCAPTURE YUV FRAMES\n--------------------------\n";

    std::shared_ptr<MMapBuffer> raw_frame;
    std::shared_ptr<cv::Mat> raw_mat;

    for (int i = 0; i < 1; i++)
    {
        camera.grab();
        camera.read(raw_frame);
        std::cout << (void *) raw_frame->start << std::endl;
        rawToFile("../out/raw.raw", raw_frame);

        camera.read(raw_mat, CV_8UC1);
        saveToFile("../out/notpng.png", raw_mat);
    }

    // std::cout << "\nCAPTURE JPG FRAME\n--------------------------\n";

    // camera.setFormat(960, 720, V4L2_PIX_FMT_MJPEG);
    // grabFrame(raw_frame, camera, 5);

    return 0;
}
