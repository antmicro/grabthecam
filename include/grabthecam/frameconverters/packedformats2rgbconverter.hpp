#pragma once

#include "grabthecam/frameconverter.hpp"

namespace grabthecam
{

typedef enum PackedFormatEnum
{
    PACKED_RGB332,
    PACKED_RGB565,
    PACKED_RGBA444,
    PACKED_BGRA444,
    PACKED_ARGB444,
    PACKED_ABGR444,
    PACKED_RGBA555,
    PACKED_BGRA555,
    PACKED_ARGB555,
    PACKED_ABGR555

} PackedFormatEnum;

class PackedFormats2RGBconverter : public FrameConverter
{
public:
    PackedFormats2RGBconverter(PackedFormatEnum type, int input_format = CV_8UC2)
    {
        this->input_format = input_format;
        this->type = type;
    }
    cv::Mat convert(cv::Mat src) override;

private:
    PackedFormatEnum type;
};

}; // namespace grabthecam
