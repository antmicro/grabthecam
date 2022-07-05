#include "Camera.hpp"

int main(int argc, char const *argv[])
{
  std::unique_ptr<v4l2_capability> cap = std::make_unique<v4l2_capability>();

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
  camera.capture("photo.jpg");

  camera.release();
  return 0;
}
