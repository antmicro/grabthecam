#include "camera-capture/cameracapture.hpp"

template <Numeric T> int CameraCapture::set(int property, T value, bool warning)
{
    v4l2_ext_control ctrl[1];
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl[0].value = value;

    return setCtrl(property, ctrl, warning);
}

template <Numeric T> int CameraCapture::get(int property, T *value, bool current) const
{
    v4l2_ext_controls ctrls;
    int res = getCtrls(property, current, ctrls);
    *value = ctrls.controls[0].value;

    return res;
}
