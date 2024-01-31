#include "cxxopts/cxxopts.hpp"
#include "farshow/framesender.hpp"
#include "farshow/streamexception.hpp"
#include "grabthecam/cameracapture.hpp"
#include "grabthecam/frameconverters/anyformat2bgrconverter.hpp"
#include "grabthecam/frameconverters/bayer2bgrconverter.hpp"
#include "grabthecam/frameconverters/yuv2bgrconverter.hpp"
#include "grabthecam/pixelformatsinfo.hpp"
#include <csignal>
#include <opencv2/imgproc.hpp>
#include <thread>

#include "grabthecam/utils.hpp"

bool app_running = true;

void appStopHandler(int signum)
{
    app_running = false;
    std::signal(signum, SIG_DFL);
}

/**
 * User's preferred configuration
 */
typedef struct Config
{
    std::string camera_filename;           ///< Path to the camera file
    std::string type = "";                 ///< Raw frame type
    std::vector<int> dims = {};            ///< Frame width and height
    unsigned int pix_format = 0;           ///< Raw frame type – v4l2 code
    bool use_trigger = false;              ///< Whether to set up and use an external trigger
    std::string ip;                        ///< IP Address for the farshow streamer
    uint16_t port;                         ///< Port for the farshow streamer
    std::string stream_name;               ///< Name of the farshow stream
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
    cxxopts::Options options(argv[0], "A simple camera frame streaming application with farshow.");

    // clang-format off
    options.add_options()
        ("c, camera", "Filename of a camera device",
                cxxopts::value(config.camera_filename)->default_value("/dev/video0"))
        ("t, type", "Frame type in V4L2 FourCC format (eg. RG12, RGGB) or compressed format type (supported: JPG, PNG)",
                cxxopts::value(config.type))
        ("d, dims", "Frame width and height (eg. `960,720`)",
                cxxopts::value(config.dims))
        ("a, address", "IP address for the farshow streamer",
                cxxopts::value(config.ip))
        ("p, port", "Port for the farshow streamer",
                cxxopts::value(config.port))
        ("n, name", "Stream name",
                cxxopts::value(config.stream_name)->default_value("grabthecam_demo stream"))
        // The only working syntax for save and load is --save=value and --load=value, 
        // it is a known limitation of cxxopts described in 
        // https://github.com/jarro2783/cxxopts/issues/210 where implicit values have to
        // be assigned through the `=` sign or will otherwise be ignored
        ("s, save", "Save configuration to the file. You can provide the filename or the "
             ".pyvidctrl_<driver_name> file will be used",
                cxxopts::value(config.saveConfig)->implicit_value(""))
        ("l, load", "Load the configuration from file. You can provide the filename or the"
            " .pyvidctrl_<driver_name> file will be used",
                cxxopts::value(config.loadConfig)->implicit_value(""))
        ("use_trigger", "Use any trigger source provided in the configuration file")
        ("h, help", "Print usage");
    // clang-format on

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
            if (config.type.length() == 4)
            {
                config.pix_format = grabthecam::convertToV4l2Fourcc(config.type);
            }
            else if (config.type == "JPG")
            {
                config.pix_format = V4L2_PIX_FMT_MJPEG;
            }
            else
            {
                throw(std::out_of_range("Wrong format"));
            }
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

    if (result.count("use_trigger"))
    {
        config.use_trigger = true;
    }

    return config;
}

int main(int argc, char const *argv[])
{
    // SET UP THE CAMERA
    Config conf = parseOptions(argc, argv);                 ///< user's configuration
    grabthecam::CameraCapture camera(conf.camera_filename); ///< cameracapture object
    farshow::FrameSender streamer(conf.ip, conf.port);

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
    if (camera.getTriggerInfo().has_value() && conf.use_trigger)
    {
        std::cout << "\nThe camera is now waiting for an external trigger."
                     "\nIn order to save the frame, please set it off via "
                     "an external tool\nor provide a triggering implementation "
                     "to CameraCapture::triggerFrame and call it.\n";
        camera.enableTrigger();
    }

    auto f = [&streamer, conf](cv::Mat foo) { streamer.sendFrame(foo, conf.stream_name); };
    std::signal(SIGINT, appStopHandler);

    // CAPTURE FRAMES
    if (camera.hasConverter())
    {
        cv::Mat processed_frame = camera.capture();
        while (app_running)
        {
            std::thread sender(f, processed_frame);
            processed_frame = camera.capture();
            sender.join();
        }
    }
    else
    {
        throw grabthecam::CameraException(std::to_string(conf.pix_format) +
                                          " is not convertable from RAW via CV2. Please specify another pixel format.");
    }
    return 0;
}
