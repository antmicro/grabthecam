#include "camera-capture/camera.hpp"
#include "camera-capture/frames/yuvframe.hpp"
#include "camera-capture/frames/bayerframe.hpp"
#include <sstream>

void grab_frame(uframe_ptr &frame, Camera &camera, int i)
{
    std::stringstream filename;
    frame = std::make_unique<YuvFrame>(cv::COLOR_YUV2BGR_YUY2);
    //frame = std::make_unique<BayerFrame>(cv::COLOR_BayerBG2BGR);
    camera.capture(frame, 0, 1);

    // save frames
    filename << "../out/raw_" << i << ".raw";
    frame->rawFrameToFile(filename.str());
    std::cout << "Raw frame saved\n";
    filename.str("");
    filename.clear();

    filename << "../out/processed_" << i << ".png";
    frame->processedFrameToFile(filename.str());
    std::cout << "Processed frame saved\n";
    filename.str("");
    filename.clear();
}

int main(int argc, char const *argv[])
{
    BayerFrame bf = BayerFrame(cv::COLOR_BayerBG2BGR);
    bf.readFromFile("../res/RGGB_1000_750", 1000, 750, CV_8UC1);
    bf.rawFrameToFile("../out/raw_bayer.raw");
    bf.processedFrameToFile("../out/processed_bayer.png");

    YuvFrame yf = YuvFrame(cv::COLOR_YUV2BGR_UYVY);
    bf.readFromFile("../res/UYVY_1000_750", 1000, 750, CV_8UC1);
    bf.rawFrameToFile("../out/raw_yuv.raw");
    bf.processedFrameToFile("../out/processed_yuv.png");


    // get camera capabilities
    ucap_ptr cap = std::make_unique<v4l2_capability>();

    Camera camera("/dev/video0");
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

    // get frame
    uframe_ptr frame;

    for (int i = 0; i < 3; i++)
    {
        grab_frame(frame, camera, i);
    }

    frame = nullptr;

    camera.setFormat(960, 720, V4L2_PIX_FMT_MJPEG);

    grab_frame(frame, camera, 5);

    return 0;
}
