#include <sstream>

#include "camera-capture/cameracapture.hpp"
#include "camera-capture/frameconverters/raw2bayerconverter.hpp"
#include "camera-capture/frameconverters/raw2yuvconverter.hpp"
#include "camera-capture/frameconverters/anyformat2bgrconverter.hpp"
#include "cxxopts/cxxopts.hpp"
#include <filesystem>            // checking if the directory exists
#include <opencv2/imgcodecs.hpp> //imwrite
#include <opencv2/imgproc.hpp>

/**
 * Check if directories exist and create them if necessary
 *
 * @param filename Path for filename which should be present in filesystem
 */
void createDirectories(std::string filename);

/**
 * Save raw frame to file
 *
 * Saves the frame as a sequence of bytes
 *
 * @param filename Where to save the file
 * @param frame The raw frame to save
 */
void rawToFile(std::string filename, std::shared_ptr<MMapBuffer> frame);

/**
 * Save processed frame to file
 *
 * Saves the file using opencv method
 *
 * @param filename Where to save the file
 * @param frame The frame to save
 */
void saveToFile(std::string filename, std::shared_ptr<cv::Mat> frame);

/**
 * Set frame converter
 *
 * Set proper converter for camera capture according to given pixel format
 *
 * @param camera CameraCapture object, in which converter will be set
 * @param pix_format Pixel format to determine converter type
 * @param raw Return parameter whether frame needs processing, or should be read as MMapBuffer
 */
void setConverter(CameraCapture &camera, unsigned int pix_format, bool &raw);

typedef struct Config
{
    std::string camera_filename;
    std::string out_filename;
    std::string type;
    std::vector<int> dims;
    unsigned int pix_format;
} Config;

/**
 * Parse command line options
 *
 * @param argc Arguments counter
 * @param argv Arguments values
 */
Config parseOptions(int argc, char const *argv[]);
