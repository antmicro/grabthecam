# include "camera-capture/cameracapture.hpp"

template <typename T>
int CameraCapture::set(int ioctl, T *value)
{
    std::cout << "set\n";
    int res = v4l2_ioctl(this->fd, ioctl, value);
    if (res !=0)
    {
        std::string default_message = "Setting property failed: " + std::to_string(errno) + " (" + strerror(errno) + ")";
        switch(errno)
        {
            case 84:
                throw CameraException(default_message + "\nCheck if your change is compatible with other camera settings.");
                break;
            default:
                throw CameraException(default_message);
        }
    }
    return res;
}

template<Numeric T>
int CameraCapture::set(int property, T value)
{
    std::cout << value << " numeric set\n";
    v4l2_control c;
    c.id = property;
    c.value = value;
    return this->set(VIDIOC_S_CTRL, &c);
}

template<typename T>
int CameraCapture::get(int ioctl, T *value) const
{
    int res = v4l2_ioctl(this->fd, ioctl, value);
    if (res !=0)
    {
        std::string default_message = "Getting property failed: " + std::to_string(errno) + " (" + strerror(errno) + ")";
        switch(errno)
        {
            case 22:
                throw CameraException(default_message + "\nCheck if your stucture is valid and you've filled all required fields.");
                break;
            case 25:
                throw CameraException(default_message + "\nIf it's a property, use the overloaded function");
                break;
            case 13:
                std::cerr <<"err 13 you shell not pass\n"; //TODO delete
                break;
            default:
                throw CameraException(default_message);
        }
    }
    return res;
}




template<Numeric T>
int CameraCapture::get(int property, T *value) const
{
    int res;
    struct v4l2_control control;

    res = queryProperty(property);
    if (res == 0)
    {
        v4l2_ext_control ctrl[1];
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl[0].id = property;
        ctrl[0].size = 0;

        v4l2_ext_controls ctrls;
        memset(&ctrls, 0, sizeof(ctrls));
        ctrls.which = V4L2_CTRL_WHICH_CUR_VAL;
        ctrls.count = 1;
        ctrls.controls = ctrl;
        res = this->get(VIDIOC_G_EXT_CTRLS, &ctrls);
        *value = ctrls.controls[0].value;
    }
    return res;
}
