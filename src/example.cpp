#include "cxxopts/cxxopts.hpp"
#include "grabthecam/cameracapture.hpp"
#include "grabthecam/cameracapturetemplates.hpp"
#include "grabthecam/frameconverters/anyformat2bgrconverter.hpp"
#include "grabthecam/frameconverters/bayer2bgrconverter.hpp"
#include "grabthecam/frameconverters/yuv2bgrconverter.hpp"
#include "grabthecam/pixelformatsinfo.hpp"
#include <opencv2/imgproc.hpp>

#include "grabthecam/utils.hpp"

/**
 * User's preferred configuration
 */
typedef struct Config
{
    std::string camera_filename;           ///< Path to the camera file
    std::string out_filename;              ///< Where to save the file
    std::string type = "";                 ///< Raw frame type
    std::vector<int> dims = {};            ///< Frame width and height
    unsigned int pix_format = 0;           ///< Raw frame type – v4l2 code
    std::optional<std::string> saveConfig; ///< Where to save the configuration
    std::optional<std::string> loadConfig; ///< Where to load the configuration from
} Config;

/**
 * Parse command line options
 *
 * @param argc Arguments counter
 * @param argv Arguments values
 */
Config parseOptions(int argc, char const *argv[])
{
    Config config;
    cxxopts::ParseResult result;

    // Set available options
    cxxopts::Options options(argv[0], "A demo for grabthecam – lightweight, easily adjustable library for "
                                      "managing v4l cameras and capturing frames.");

    // clang-format off
    options.add_options()
        ("c, camera", "Filename of a camera device",
                cxxopts::value(config.camera_filename)->default_value("/dev/video0"))
        ("t, type", "Frame type (allowed values: YUYV, JPG, BGRA, AR24, RGGB, RG12)",
                cxxopts::value(config.type))
        ("o, out", "Path to save the frame",
                cxxopts::value(config.out_filename))
        ("d, dims", "Frame width and height (eg. `960,720`)",
                cxxopts::value(config.dims))
        // The only working syntax for save and load is --save=value and --load=value, 
        // it is a known limitation of cxxopts described in 
        // https://github.com/jarro2783/cxxopts/issues/210 where implicit values have to
        // be assigned through the `=` sign or will otherwise be ignored
        ("save", "Save configuration to the file. You can provide the filename or the "
             ".pyvidctrl_<driver_name> file will be used",
                cxxopts::value(config.saveConfig)->implicit_value(""))
        ("load", "Load the configuration from file. You can provide the filename or the"
            " .pyvidctrl_<driver_name> file will be used",
                cxxopts::value(config.loadConfig)->implicit_value(""))("h, help", "Print usage");
    // clang-format on

    std::unordered_map<std::string, unsigned int> pix_formats = {
        {"YUYV", V4L2_PIX_FMT_YYUV},   {"JPG", V4L2_PIX_FMT_MJPEG},   {"BGRA", V4L2_PIX_FMT_ABGR32},
        {"AR24", V4L2_PIX_FMT_ABGR32}, {"RGGB", V4L2_PIX_FMT_SRGGB8}, {"RG10", V4L2_PIX_FMT_SRGGB10},
        {"RG12", V4L2_PIX_FMT_SRGGB12}};

    // Get command line parameters and parse them
    try
    {
        result = options.parse(argc, argv);
    }
    catch (cxxopts::OptionException e)
    {
        std::cerr << std::endl
                  << "\033[31mError while parsing command line arguments: " << e.what() << "\033[0m" << std::endl
                  << std::endl;
        std::cout << options.help() << std::endl;
        exit(1);
    }
    catch (grabthecam::CameraException e)
    {
        std::cerr << "\033[31m" << e.what() << "\033[0m" << std::endl << std::endl;
        std::cout << options.help() << std::endl;
        exit(1);
    }

    if (result.count("type"))
    {
        try
        {
            config.pix_format = pix_formats.at(config.type);
        }
        catch (std::out_of_range e)
        {
            std::cerr << std::endl
                      << "\033[31mError while parsing command line arguments: Wrong value '" << config.type
                      << "' for parameter 'type'\033[0m" << std::endl
                      << std::endl;
            std::cout << options.help() << std::endl;
            exit(1);
        }
    }

    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    if (result.count("out") == 0)
    {
        config.out_filename = "";
    }

    return config;
}

int main(int argc, char const *argv[])
{
    // SET UP THE CAMERA
    Config conf = parseOptions(argc, argv);                 ///< user's configuration
    grabthecam::CameraCapture camera(conf.camera_filename); ///< cameracapture object

    camera.printControls();

    // adjust camera settings
    if (!conf.dims.empty())
    {
        camera.setFormat(conf.dims[0], conf.dims[1], conf.pix_format); // set frame format
    }
    else
    {
        camera.setFormat(0, 0, conf.pix_format);
    }
    auto format = camera.getFormat(); ///< Actually set frame format
    double time_perframe;

    ///////////////////////////////////////////////////////////////////////////////////////////////

    std::cout << "Format set to " << conf.type << " " << std::get<0>(format) << " x " << std::get<1>(format)
              << std::endl;

    if (conf.loadConfig.has_value())
    {
        std::string filename = camera.loadConfig(*conf.loadConfig);
        std::cout << "Configuration loaded from " << filename << std::endl;
    }

    if (conf.saveConfig.has_value())
    {
        std::string filename = camera.saveConfig(*conf.saveConfig);
        std::cout << "Configuration saved to " << filename << std::endl;
    }

    // detect if trigger information was set
    if (camera.getTriggerInfo().has_value())
    {
        camera.setTrigger(camera.getTriggerInfo().value());
    }

    // CAPTURE FRAME
    if (conf.out_filename != "")
    {
        if (camera.hasConverter())
        {
            cv::Mat processed_frame = camera.capture();                          ///< captured frame
            grabthecam::saveToFile(conf.out_filename + ".png", processed_frame); // save it
        }
        else
        {
            std::shared_ptr<grabthecam::MMapBuffer> raw_frame;         ///< Frame fetched from the camera
            camera.grab();                                             // fetch the frame to camera's buffer 0
            camera.read(raw_frame);                                    // read content from the buffer
            rawToFile(conf.out_filename + "." + conf.type, raw_frame); // save it
        }
    }
    return 0;
}
