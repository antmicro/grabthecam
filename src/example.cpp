#include "../includes/Camera.hpp"

int main(int argc, char const *argv[])
{
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

    camera.setFormat(1024, 1024, V4L2_PIX_FMT_MJPEG);
    camera.set(V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_MANUAL);

    double val;
    camera.get(V4L2_CID_EXPOSURE_AUTO, val);
    std::cout << "Value of V4L2_CID_EXPOSURE_AUTO: " << val << std::endl;

    uframe_ptr frame;
    camera.capture(frame, NULL);

    frame -> rawFrameToFile("../out/photo.jpg");
    return 0;
}
