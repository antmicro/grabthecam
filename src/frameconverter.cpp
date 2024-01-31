// Copyright 2022-2024 Antmicro <www.antmicro.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "grabthecam/frameconverter.hpp"

namespace grabthecam
{

cv::Mat FrameConverter::convert(std::shared_ptr<MMapBuffer> src, int src_dtype, int width, int height)
{
    cv::Mat raw_frame = cv::Mat(height, width, src_dtype, src->start);
    return convert(raw_frame);
}

}; // namespace grabthecam
