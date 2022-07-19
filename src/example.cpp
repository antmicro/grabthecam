#include <sstream>

#include "camera-capture/cameracapture.hpp"
#include "camera-capture/frameconverters/raw2yuvconverter.hpp"
#include "camera-capture/frameconverters/raw2bayerconverter.hpp"

void grab_frame(std::unique_ptr<RawFrame> &frame, CameraCapture &camera, int i=0)
{
    std::stringstream filename;
    frame = std::make_unique<RawFrame>(CV_8UC2);
    camera.capture(frame, 0, 1);

    // save frame
    filename << "../out/raw_" << i << ".raw";
    frame->saveToFile(filename.str());
    std::cout << "Raw frame saved\n";
    filename.str("");
    filename.clear();

    auto r2yconv = Raw2YuvConverter(cv::COLOR_YUV2BGR_YUY2);
    Frame processed_frame = r2yconv.convert(frame.get());

    // save frame
    filename << "../out/processed_" << i << ".png";
    processed_frame.saveToFile(filename.str());
    std::cout << "Processed frame saved\n";
    filename.str("");
    filename.clear();
}

int main(int argc, char const *argv[])
{
    std::cout << "READ RAW IMAGE FROM FILE\n--------------------------\n";

    auto r2bconv = Raw2BayerConverter(cv::COLOR_BayerBG2BGR);
    RawFrame bayerFrame;
    bayerFrame.readFromFile("../res/RGGB_1000_750", 1000, 750);
    bayerFrame.saveToFile("../out/raw_bayer.raw");
    Frame processed = r2bconv.convert(&bayerFrame);
    processed.saveToFile("../out/processed_bayer.png");

    std::cout << "\nSET CAMERA\n--------------------------\n";
    // get camera capabilities
    auto cap = std::make_unique<v4l2_capability>();

    CameraCapture camera("/dev/video0");
    camera.getCapabilities(cap);

    if (!(cap->capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        std::cerr<<"This is not a video capture device\n";
        exit (EXIT_FAILURE);
    }

    if (!(cap->capabilities & V4L2_CAP_STREAMING))
    {
        std::cerr<<"V4L2_CAP_STREAMING failed\n";
        exit (EXIT_FAILURE);
    }

    // adjust camera settings
    camera.setFormat(960, 720, V4L2_PIX_FMT_YYUV);
    camera.set(V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_MANUAL);

    double val;
    camera.get(V4L2_CID_EXPOSURE_AUTO, val);
    std::cout << "Value of V4L2_CID_EXPOSURE_AUTO: " << val << std::endl;

    std::cout << "\nCAPTURE YUV FRAMES\n--------------------------\n";

    std::unique_ptr<RawFrame> raw_frame;

    for (int i = 0; i < 3; i++)
    {
        grab_frame(raw_frame, camera, i);
    }


    std::cout << "\nCAPTURE JPG FRAME\n--------------------------\n";

    camera.setFormat(960, 720, V4L2_PIX_FMT_MJPEG);
    grab_frame(raw_frame, camera, 5);

    return 0;
}
