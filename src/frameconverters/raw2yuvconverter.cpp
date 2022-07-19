#include "camera-capture/frameconverters/raw2yuvconverter.hpp"
#include "camera-capture/rawframe.hpp"

Raw2YuvConverter::Raw2YuvConverter(int code, int destMatType) : code(code), destMatType(destMatType)
{}

Frame Raw2YuvConverter::convert(Frame *src)
{
    RawFrame* raw = dynamic_cast<RawFrame*>(src);
    if(raw == nullptr)
    {
        throw CameraException("This is not a raw frame.");
    }
    return Frame(convertMatrix(raw->getMatrix()));
}

cv::Mat Raw2YuvConverter::convertMatrix(cv::Mat src)
{
    // std::cout << "YUV\n";
    cv::Mat processed_frame = cv::Mat(src.rows, src.cols, destMatType);
    cv::cvtColor(src, processed_frame, code);
    return processed_frame;
}
