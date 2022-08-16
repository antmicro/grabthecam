#include "camera-capture/cameracapture.hpp"

template <Numeric T> int CameraCapture::set(int property, T value)
{
    v4l2_ext_control ctrl[1];
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl[0].value = value;

    // TODO: handle different control types
    // https://www.kernel.org/doc/html/v5.0/media/uapi/v4l/vidioc-g-ext-ctrls.html#description
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

    return setCtrl(property, ctrl);
}

template <Numeric T> int CameraCapture::get(int property, T *value, bool current) const
{
    v4l2_ext_controls ctrls;
    int res = getCtrls(property, current, ctrls);
    *value = ctrls.controls[0].value;

    return res;
}
