#include <sstream>

#include "camera-capture/frameconverters/raw2yuvconverter.hpp"
#include "camera-capture/cameracapture.hpp"
#include <filesystem> // checking if the directory exists
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp> //imwrite

void rawToFile(std::string filename, std::shared_ptr<MMapBuffer> info)
{
    std::cout << "Saving " << filename << std::endl;
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
    std::cout << "Saving " << filename << std::endl;

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
    std::shared_ptr<cv::Mat> raw_mat[3];
    std::shared_ptr<cv::Mat> processed_frame;

    std::shared_ptr<FrameConverter> converter = std::make_shared<Raw2YuvConverter>(cv::COLOR_YUV2BGR_YUY2);
    camera.setConverter(converter);

    for (int i = 0; i < 3; i++)
    {
        // camera.grab();
        // camera.read(raw_frame);

        // rawToFile("../out/raw_" + std::to_string(i) + ".raw", raw_frame);
        // processed_frame = std::make_shared<cv::Mat> (converter->convertMatrix(raw_frame, CV_8UC2, 960, 720));

        processed_frame = std::make_shared<cv::Mat>(camera.capture(CV_8UC2));
        saveToFile("../out/processed_frame_" + std::to_string(i)+".png", processed_frame);
    }


    std::cout << "\nCAPTURE JPG FRAME\n--------------------------\n";

    camera.setConverter(nullptr);
    camera.setFormat(960, 720, V4L2_PIX_FMT_MJPEG);
    camera.grab();
    camera.read(raw_frame);
    rawToFile("../out/jpg.jpg", raw_frame);
    return 0;
}
