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
    std::cout<<"get\n";
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
void CameraCapture::get(int property, T *value) const
{
    std::cout << *value << " numeric get\n";
    //TODO:return res

    // struct v4l2_queryctrl queryctrl;
    // struct v4l2_control control;

    // memset(&queryctrl, 0, sizeof(queryctrl));
    // queryctrl.id = property;

    // if (-1 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {
    //         if (errno != EINVAL) {
    //                     perror("VIDIOC_QUERYCTRL");
    //                             exit(EXIT_FAILURE);
    //         } else {
    //                     printf("propertry is not supportedn");
    //         }
    // } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
    //         printf("property is not supportedn (disabled)");
    // } else {
    //     memset(&control, 0, sizeof (control));
    //     control.id = property;
    //     control.value = queryctrl.default_value;

    //     if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control)) {
    //         perror("VIDIOC_S_CTRL"); //TODO: change
    //         exit(EXIT_FAILURE);
    //     }
    //     }
    // *value = control.value;

    v4l2_ext_control c[1];
    memset(&c, 0, sizeof(c));
    c[0].id = property;
    c[0].size = 0;

    v4l2_ext_controls ctls;
    memset(&ctls, 0, sizeof(ctls));
    ctls.which = V4L2_CTRL_WHICH_CUR_VAL;
    ctls.count = 1;
    ctls.controls = c;
    this->get(VIDIOC_G_EXT_CTRLS, &ctls);
    *value = ctls.controls[0].value;
