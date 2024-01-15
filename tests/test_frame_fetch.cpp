#include "grabthecam/cameracapture.hpp"
#include "grabthecam/cameracapturetemplates.hpp"
#include "grabthecam/frameconverters/anyformat2bgrconverter.hpp"
#include "grabthecam/frameconverters/packedformats2rgbconverter.hpp"
#include "grabthecam/frameconverters/bayer2bgrconverter.hpp"
#include "grabthecam/frameconverters/yuv2bgrconverter.hpp"
#include "grabthecam/pixelformatsinfo.hpp"
#include <opencv2/imgproc.hpp>

#include "grabthecam/utils.hpp"

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "Please give a video device path as an argument\n";
        return 1;
    }
    grabthecam::CameraCapture camera(argv[1]);
    for (const auto& entry : grabthecam::formats_info) {
        camera.setFormat(1280, 720, entry.first);
        cv::Mat frame = camera.capture();
        char name[5] = {
            (char)((uint32_t)(entry.first       ) & 0xff),
            (char)((uint32_t)(entry.first >> 8  ) & 0xff),
            (char)((uint32_t)(entry.first >> 16 ) & 0xff),
            (char)((uint32_t)(entry.first >> 24 ) & 0xff),
            0
        };
        std::string filename = "frame_" + std::string(name) + ".png";
        grabthecam::saveToFile(filename, frame);
    }
    return 0;
}
