#pragma once

#include "grabthecam/frameconverters/anyformat2bgrconverter.hpp"
#include "grabthecam/frameconverters/bayer2bgrconverter.hpp"
#include "grabthecam/frameconverters/yuv2bgrconverter.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <linux/videodev2.h>

namespace grabthecam
{

static std::unordered_map<unsigned int, std::function<std::shared_ptr<FrameConverter>()>> formats_info = {
    {V4L2_PIX_FMT_YYUV, [] { return std::make_shared<Yuv2BGRConverter>(cv::COLOR_YUV2BGR_YUY2); }},
    {V4L2_PIX_FMT_YUYV, [] { return std::make_shared<Yuv2BGRConverter>(cv::COLOR_YUV2BGR_YUY2); }},
    {V4L2_PIX_FMT_ABGR32,
     [] { return std::make_shared<AnyFormat2BGRConverter>(cv::COLOR_BGRA2RGB, CV_8UC3, CV_8UC4); }},
    {V4L2_PIX_FMT_SRGGB8, [] { return std::make_shared<Bayer2BGRConverter>(cv::COLOR_BayerRG2BGR); }},
    {V4L2_PIX_FMT_SRGGB12,
     [] { return std::make_shared<Bayer2BGRConverter>(cv::COLOR_BayerRG2BGR, CV_16UC1, CV_16UC3); }},
    {V4L2_PIX_FMT_SRGGB10,
     [] { return std::make_shared<Bayer2BGRConverter>(cv::COLOR_BayerRG2BGR, CV_16UC1, CV_16UC3); }}}; ///< Information
                                                                                                       ///< about
                                                                                                       ///< converters
                                                                                                       ///< and input
                                                                                                       ///< formats
                                                                                                       ///< assigned to
                                                                                                       ///< pixel
                                                                                                       ///< formats

};
