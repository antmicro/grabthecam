#include "grabthecam/frameconverters/packedformats2rgbconverter.hpp"
#include "grabthecam/cameracapture.hpp"

#include <opencv2/imgproc.hpp>

namespace grabthecam
{

cv::Mat PackedFormats2RGBconverter::convert(cv::Mat src)
{
    cv::Mat rgb_image(src.rows, src.cols, CV_8UC3);
    switch (this->type)
    {
    case PACKED_RGB332:
    {
        // Make sure that for camera.capture() there is a format argument
        // set to CV_8UC1
        if (src.type() != CV_8UC1)
        {
            throw(CameraException("Please set the correct format for camera.capture()\n"));
        }
        for (int y = 0; y < src.rows; ++y)
        {
            for (int x = 0; x < src.cols; ++x)
            {
                uint8_t pixel = src.at<uint8_t>(y, x);
                uint8_t r = ((pixel >> 5) & 0x07) * 255 / 7;
                uint8_t g = ((pixel >> 2) & 0x07) * 255 / 7;
                uint8_t b = ((pixel)&0x03) * 255 / 3;
                rgb_image.at<cv::Vec3b>(y, x) = cv::Vec3b(b, g, r);
            }
        }
        break;
    }
    case PACKED_RGB565:
    {
        // Make sure that for camera.capture() there is a format argument
        // set to CV_16UC1
        if (src.type() != CV_16UC1)
        {
            throw(CameraException("Please set the correct format for camera.capture()\n"));
        }
        for (int y = 0; y < src.rows; ++y)
        {
            for (int x = 0; x < src.cols; ++x)
            {
                uint16_t pixel = src.at<uint16_t>(y, x);
                uint8_t r = ((pixel >> 11) & 0x1f) * 255 / 0x1f;
                uint8_t g = ((pixel >> 5) & 0x3f) * 255 / 0x3f;
                uint8_t b = ((pixel)&0x01f) * 255 / 0x1f;
                rgb_image.at<cv::Vec3b>(y, x) = cv::Vec3b(b, g, r);
            }
        }
        break;
    }
    case PACKED_RGBA444:
    case PACKED_BGRA444:
    {
        // Make sure that for camera.capture() there is a format argument
        // set to CV_16UC1
        if (src.type() != CV_16UC1)
        {
            throw(CameraException("Please set the correct format for camera.capture()\n"));
        }
        for (int y = 0; y < src.rows; ++y)
        {
            for (int x = 0; x < src.cols; ++x)
            {
                uint16_t pixel = src.at<uint16_t>(y, x);

                uint8_t r = ((pixel >> 12) & 0x0F) * 255 / 0x0F;
                uint8_t g = ((pixel >> 8) & 0x0F) * 255 / 0x0F;
                uint8_t b = ((pixel >> 4) & 0x0F) * 255 / 0x0F;
                if (this->type == PACKED_BGRA444)
                {
                    std::swap(r, b);
                }
                rgb_image.at<cv::Vec3b>(y, x) = cv::Vec3b(b, g, r);
            }
        }
        break;
    }
    case PACKED_ARGB444:
    case PACKED_ABGR444:
    {
        // Make sure that for camera.capture() there is a format argument
        // set to CV_16UC1
        if (src.type() != CV_16UC1)
        {
            throw(CameraException("Please set the correct format for camera.capture()\n"));
        }
        for (int y = 0; y < src.rows; ++y)
        {
            for (int x = 0; x < src.cols; ++x)
            {
                uint16_t pixel = src.at<uint16_t>(y, x);

                uint8_t r = ((pixel >> 8) & 0x0F) * 255 / 0x0F;
                uint8_t g = ((pixel >> 4) & 0x0F) * 255 / 0x0F;
                uint8_t b = ((pixel)&0x0F) * 255 / 0x0F;
                if (this->type == PACKED_ABGR444)
                {
                    std::swap(r, b);
                }
                rgb_image.at<cv::Vec3b>(y, x) = cv::Vec3b(b, g, r);
            }
        }
        break;
    }
    case PACKED_RGBA555:
    case PACKED_BGRA555:
    {
        // Make sure that for camera.capture() there is a format argument
        // set to CV_16UC1
        if (src.type() != CV_16UC1)
        {
            throw(CameraException("Please set the correct format for camera.capture()\n"));
        }
        for (int y = 0; y < src.rows; ++y)
        {
            for (int x = 0; x < src.cols; ++x)
            {
                uint16_t pixel = src.at<uint16_t>(y, x);

                uint8_t r = ((pixel >> 11) & 0x1F) * 255 / 0x1F;
                uint8_t g = (((pixel >> 5) & 0x3) | (((pixel >> 8) & 0x7) << 2)) * 255 / 0x1F;
                uint8_t b = ((pixel >> 1) & 0x1F) * 255 / 0x1F;
                if (this->type == PACKED_BGRA555)
                {
                    std::swap(r, b);
                }
                rgb_image.at<cv::Vec3b>(y, x) = cv::Vec3b(b, g, r);
            }
        }
        break;
    }
    case PACKED_ARGB555:
    case PACKED_ABGR555:
    {
        // Make sure that for camera.capture() there is a format argument
        // set to CV_16UC1
        if (src.type() != CV_16UC1)
        {
            throw(CameraException("Please set the correct format for camera.capture()\n"));
        }
        for (int y = 0; y < src.rows; ++y)
        {
            for (int x = 0; x < src.cols; ++x)
            {
                uint16_t pixel = src.at<uint16_t>(y, x);

                uint8_t r = ((pixel >> 10) & 0x1F) * 255 / 0x1F;
                uint8_t g = (((pixel >> 5) & 0x7) | (((pixel >> 8) & 0x3) << 3)) * 255 / 0x1F;
                uint8_t b = ((pixel)&0x1F) * 255 / 0x1F;
                if (this->type == PACKED_ABGR555)
                {
                    std::swap(r, b);
                }
                rgb_image.at<cv::Vec3b>(y, x) = cv::Vec3b(b, g, r);
            }
        }
        break;
    }
    }
    return rgb_image;
}

}; // namespace grabthecam
