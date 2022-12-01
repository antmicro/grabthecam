#include "grabthecam/cameracapture.hpp"

namespace grabthecam
{

template <Numeric T> void CameraCapture::set(int property, T value, bool warning)
{
    v4l2_ext_control ctrl[1];
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl[0].value = value;

    setCtrl(property, ctrl, warning);
}

template <Numeric T> void CameraCapture::get(int property, T &value, bool current) const
{
    v4l2_ext_controls ctrls;
    getCtrls(property, current, ctrls);
    value = ctrls.controls[0].value;
}

};
