#pragma once

#include "grabthecam/frameconverters/anyformat2bgrconverter.hpp"
#include "grabthecam/frameconverters/bayer2bgrconverter.hpp"
#include "grabthecam/frameconverters/yuv2bgrconverter.hpp"
#include "grabthecam/frameconverters/packedformats2rgbconverter.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <map>
#include <linux/videodev2.h>

namespace grabthecam
{

/**
 * Parse command line options
 *
 * @param name Four character name of the V4L2 format
 */
uint32_t convertToV4l2Fourcc(std::string name)
{
    uint32_t fourcc = 0;
    for (int i = 0; i < 4; i++)
    {
        fourcc |= (name[0] << 8 * i);
    }
    return fourcc;
}

/// Information about converters and input formats assigned to pixel formats
static std::map<unsigned int, std::function<std::shared_ptr<FrameConverter>()>> formats_info = {
    {V4L2_PIX_FMT_MJPEG, [] { return std::make_shared<AnyFormat2BGRConverter>(cv::COLOR_BGRA2BGR, CV_8UC3); }},
    {V4L2_PIX_FMT_RGB24, [] { return std::make_shared<AnyFormat2BGRConverter>(cv::COLOR_RGB2BGR, CV_8UC3); }},
    {V4L2_PIX_FMT_RGBA32, [] { return std::make_shared<AnyFormat2BGRConverter>(cv::COLOR_RGBA2BGR, CV_8UC4); }},
    {V4L2_PIX_FMT_ABGR32, [] { return std::make_shared<AnyFormat2BGRConverter>(cv::COLOR_BGRA2BGR, CV_8UC4); }},
    {V4L2_PIX_FMT_RGB332, [] { return std::make_shared<PackedFormats2RGBconverter>(grabthecam::PACKED_RGB332, CV_8UC1); }},
    {V4L2_PIX_FMT_RGB565, [] { return std::make_shared<PackedFormats2RGBconverter>(grabthecam::PACKED_RGB565, CV_16UC1); }},
    {V4L2_PIX_FMT_ARGB444, [] { return std::make_shared<PackedFormats2RGBconverter>(grabthecam::PACKED_ARGB444, CV_16UC1); }},
    {V4L2_PIX_FMT_ABGR444, [] { return std::make_shared<PackedFormats2RGBconverter>(grabthecam::PACKED_ABGR444, CV_16UC1); }},
    {V4L2_PIX_FMT_RGBA444, [] { return std::make_shared<PackedFormats2RGBconverter>(grabthecam::PACKED_RGBA444, CV_16UC1); }},
    {V4L2_PIX_FMT_BGRA444, [] { return std::make_shared<PackedFormats2RGBconverter>(grabthecam::PACKED_BGRA444, CV_16UC1); }},
    {V4L2_PIX_FMT_ARGB555, [] { return std::make_shared<PackedFormats2RGBconverter>(grabthecam::PACKED_ARGB555, CV_16UC1); }},
    {V4L2_PIX_FMT_ABGR555, [] { return std::make_shared<PackedFormats2RGBconverter>(grabthecam::PACKED_ABGR555, CV_16UC1); }},
    {V4L2_PIX_FMT_RGBA555, [] { return std::make_shared<PackedFormats2RGBconverter>(grabthecam::PACKED_RGBA555, CV_16UC1); }},
    {V4L2_PIX_FMT_BGRA555, [] { return std::make_shared<PackedFormats2RGBconverter>(grabthecam::PACKED_BGRA555, CV_16UC1); }},
    {V4L2_PIX_FMT_YUYV, [] { return std::make_shared<Yuv2BGRConverter>(cv::COLOR_YUV2BGR_YUYV); }},
    {V4L2_PIX_FMT_UYVY, [] { return std::make_shared<Yuv2BGRConverter>(cv::COLOR_YUV2BGR_UYVY); }},
    {V4L2_PIX_FMT_YVYU, [] { return std::make_shared<Yuv2BGRConverter>(cv::COLOR_YUV2BGR_YVYU); }},
    {V4L2_PIX_FMT_NV12, [] { return std::make_shared<Yuv2BGRConverter>(cv::COLOR_YUV2BGR_NV12, CV_8UC1); }},
    {V4L2_PIX_FMT_NV21, [] { return std::make_shared<Yuv2BGRConverter>(cv::COLOR_YUV2BGR_NV21, CV_8UC1); }},
    {V4L2_PIX_FMT_YVU420, [] { return std::make_shared<Yuv2BGRConverter>(cv::COLOR_YUV2BGR_YV12, CV_8UC1); }},
    {V4L2_PIX_FMT_YUV420, [] { return std::make_shared<Yuv2BGRConverter>(cv::COLOR_YUV2BGR_I420, CV_8UC1); }},
    {V4L2_PIX_FMT_SBGGR8, [] { return std::make_shared<Bayer2BGRConverter>(cv::COLOR_BayerBGGR2BGR_EA, CV_8UC1, CV_8UC3); }},
    {V4L2_PIX_FMT_SGBRG8, [] { return std::make_shared<Bayer2BGRConverter>(cv::COLOR_BayerGBRG2BGR_EA, CV_8UC1, CV_8UC3); }},
    {V4L2_PIX_FMT_SRGGB16, [] { return std::make_shared<Bayer2BGRConverter>(cv::COLOR_BayerRGGB2BGR_EA, CV_16UC1, CV_16UC3); }},
    {V4L2_PIX_FMT_SRGGB8, [] { return std::make_shared<Bayer2BGRConverter>(cv::COLOR_BayerRGGB2BGR_EA, CV_8UC1, CV_8UC3); }},
    {V4L2_PIX_FMT_SRGGB10, [] { return std::make_shared<Bayer2BGRConverter>(cv::COLOR_BayerRGGB2BGR_EA, CV_16UC1, CV_8UC3); }},
    {V4L2_PIX_FMT_SRGGB12, [] { return std::make_shared<Bayer2BGRConverter>(cv::COLOR_BayerRGGB2BGR_EA, CV_16UC1, CV_8UC3); }}};

}; // namespace grabthecam
