# include "camera-capture/cameracapture.hpp"

template <typename T>
int CameraCapture::set(int ioctl, T value)
{
    std::cout<<"Template set\n";

    int res = v4l2_ioctl(this->fd, ioctl, value);
    switch(res)
    {
        case 0:
            return 0;
        default:
            throw CameraException("Setting property failed. See errno and docs for more information");
    }
    return -1;
}

template<Numeric T>
int CameraCapture::set(int property, T  value)
{
    std::cout<<"double set\n";
    v4l2_control c;
    c.id = property;
    c.value = value;
    if (v4l2_ioctl(this->fd, VIDIOC_S_CTRL, &c) != 0)
    {
        throw CameraException("Setting property failed. See errno and VIDEOC_S_CTRL docs for more information");
        return -1;
    }
    return 0;
}

