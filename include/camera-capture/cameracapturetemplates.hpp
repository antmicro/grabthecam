# include "camera-capture/cameracapture.hpp"

template <typename T>
int CameraCapture::set(int ioctl, T value)
{
    int res = v4l2_ioctl(this->fd, ioctl, value);
    switch(res)
    {
        case 0:
            return 0;
        default:
            throw CameraException("Setting property failed. See errno and docs for more information");
    }
    return res;
}

template<Numeric T>
int CameraCapture::set(int property, T  value)
{
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

template<typename T>
int CameraCapture::get(int ioctl, T value) const
{
    //TODO: T should be a pointer or reference
    //BUG: cannot pass uniqueptr
    int res = v4l2_ioctl(this->fd, ioctl, value);
    if (res != 0)
    {
        throw CameraException("Getting property failed. See errno and docs for more information");
    }
    return res;
}

template<Numeric T>
void CameraCapture::get(int property, T value) const
{
    v4l2_control c;
    c.id = property;
    this->get(VIDIOC_G_CTRL, &c);
    value = c.value;
}
