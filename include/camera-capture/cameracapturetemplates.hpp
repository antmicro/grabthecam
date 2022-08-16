# include "camera-capture/cameracapture.hpp"

template<Numeric T>
int CameraCapture::set(int property, T value)
{
    int res;

    v4l2_queryctrl queryctrl;
    res = queryProperty(property, &queryctrl);
    if (res == 0)
    {
        v4l2_ext_control ctrl[1];
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl[0].id = property;
        ctrl[0].size = 0;
        ctrl[0].value = value;

        //TODO: VIDIOC_TRY_EXT_CTRLS
        //TODO: handle different control types https://www.kernel.org/doc/html/v5.0/media/uapi/v4l/vidioc-g-ext-ctrls.html#description
        // switch(queryctrl.type)
        // {
        //     case V4L2_CTRL_TYPE_INTEGER64:
        //         ctrl[0].value64 = value;
        //         break;
        //     case V4L2_CTRL_TYPE_STRING: //In order to support this I should change the type of value.
        //         ctrl[0].string = value;
        //         break;
        //     case V4L2_CTRL_TYPE_U8:
        //         ctrl[0].p_u8 = value;
        //         break;
        //     case V4L2_CTRL_TYPE_U16:
        //         ctrl[0].p_u16 = value;
        //         break;
        //     case V4L2_CTRL_TYPE_U32:
        //         ctrl[0].p_u32 = value;
        //         break;
        //     case >= V4L2_CTRL_COMPOUND_TYPES:
        //         ctrl[0].ptr = value;
        //         break;
        // 
        //     default:
        //         ctrl[0].value = value;
        // }

        v4l2_ext_controls ctrls;
        memset(&ctrls, 0, sizeof(ctrls));
        ctrls.which = V4L2_CTRL_WHICH_CUR_VAL;
        ctrls.count = 1;
        ctrls.controls = ctrl;
        try
        {
            res = this->runIoctl(VIDIOC_S_EXT_CTRLS, &ctrls);
        }
        catch (CameraException e)
        {
            switch(e.error_code)
            {
                case EACCES:
                    std::cerr <<"err 13 you shell not pass\n"; //TODO delete
                    break;
                case EINVAL:
                    throw CameraException("Check if your stucture is valid and you've filled all required fields.", e.error_code);
                    break;
                case ERANGE:
                    throw CameraException("Wrong parameter value. It should be between " + std::to_string(queryctrl.minimum) + " and " + std::to_string(queryctrl.maximum) + " (step: " + std::to_string(queryctrl.step) + ")");
                    break;
                case EILSEQ:
                    throw CameraException("Check if your change is compatible with other camera settings.", e.error_code);
                    break;
                default:
                    throw CameraException("", e.error_code);
             }
        }
        //TODO:
        // warning if the value is clamped?
    }
    return res;
}

template<typename T>
int CameraCapture::runIoctl(int ioctl, T *value) const
{
    int res = v4l2_ioctl(this->fd, ioctl, value);
    if (res !=0)
    {
        throw CameraException("", errno);
    }
    return res;
}




template<Numeric T>
int CameraCapture::get(int property, T *value, bool current) const
{
    int res;

    v4l2_queryctrl queryctrl;
    res = queryProperty(property, &queryctrl);
    // std::cout << "type " << queryctrl.type << std::endl;
    if (res == 0)
    {
        v4l2_ext_control ctrl[1];
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl[0].id = property;
        ctrl[0].size = 0;

        v4l2_ext_controls ctrls;
        memset(&ctrls, 0, sizeof(ctrls));
        if (current)
        {
            ctrls.which = V4L2_CTRL_WHICH_CUR_VAL;
        }
        else
        {
            ctrls.which = V4L2_CTRL_WHICH_DEF_VAL;
        }
        ctrls.count = 1;
        ctrls.controls = ctrl;
        try
        {
            res = this->runIoctl(VIDIOC_G_EXT_CTRLS, &ctrls);
            *value = ctrls.controls[0].value;
        }
        catch (CameraException e)
        {
            switch(e.error_code)
            {
               case EACCES:
                    std::cerr <<"err 13 you shell not pass\n"; //TODO delete
                    break;
               case EINVAL:
                    throw CameraException("Check if your stucture is valid and you've filled all required fields.", e.error_code);
                    break;
               case ENOSPC:
                    throw CameraException("Too small size was set. Changed to " + std::to_string(ctrl[0].size), e.error_code);
                    break;
              default:
                    throw CameraException("",  e.error_code);
             }
        }
    }
    return res;
}
