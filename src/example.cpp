#include "camera-capture/cameracapture.hpp"
#include "camera-capture/cameracapturetemplates.hpp"
#include "camera-capture/frameconverters/anyformat2bgrconverter.hpp"
#include "camera-capture/frameconverters/raw2bayerconverter.hpp"
#include "camera-capture/frameconverters/raw2yuvconverter.hpp"
#include "cxxopts/cxxopts.hpp"
#include <opencv2/imgproc.hpp>

#include "camera-capture/utils.hpp"

#include <sys/ioctl.h> //TODO: remove -- for debugging only

/**
 * User's preferred configuration
 */
typedef struct Config
{
    std::string camera_filename; ///< Path to the camera file
    std::string out_filename;    ///< Where to save the file
    std::string type;            ///< Raw frame type
    std::vector<int> dims;       ///< Frame width and height
    unsigned int pix_format;     ///< Raw frame type – v4l2 code
} Config;

/**
 * Set frame converter
 *
 * Set proper converter for camera capture according to given pixel format
 *
 * @param camera CameraCapture object, in which converter will be set
 * @param pix_format Pixel format to determine converter type
 * @param raw Return parameter whether frame needs processing, or should be read as MMapBuffer
 * @param input_format Return parameter; opencv's format for the raw frame
 */
void setConverter(CameraCapture &camera, unsigned int pix_format, bool &raw, int &input_format)
{
    std::shared_ptr<FrameConverter> converter; ///< converter to be used by the cameracapture object

    switch (pix_format)
    {
    case V4L2_PIX_FMT_YYUV:
        converter = std::make_shared<Raw2YuvConverter>(cv::COLOR_YUV2BGR_YUY2);
        input_format = CV_8UC2;
        break;

    case V4L2_PIX_FMT_ABGR32:
        converter = std::make_shared<AnyFormat2bgrConverter>(cv::COLOR_BGRA2RGB, CV_8UC3);
        input_format = CV_8UC4;
        break;

    case V4L2_PIX_FMT_SRGGB8:
        converter = std::make_shared<Raw2BayerConverter>(cv::COLOR_BayerRG2BGR, CV_8UC3);
        input_format = CV_8UC1;
        break;

    case V4L2_PIX_FMT_SRGGB12:
        converter = std::make_shared<Raw2BayerConverter>(cv::COLOR_BayerRG2BGR, CV_16UC3);
        input_format = CV_16UC1;
        break;

    case V4L2_PIX_FMT_SRGGB10:
        converter = std::make_shared<Raw2BayerConverter>(cv::COLOR_BayerRG2BGR, CV_16UC3);
        input_format = CV_16UC1;
        break;

    default:
        std::cerr << "Skipping conversion";
        converter = nullptr;
        raw = true;
        input_format = CV_8UC1;
    }

    camera.setConverter(converter);
}

void checkRequiredArgs(cxxopts::ParseResult &result, std::vector<std::string> &required)
{
    for (auto& r : required)
    {
        if (result.count(r) == 0)
        {
            throw CameraException("Error while parsing command line arguments: Parameter '" + r + "' is required");
        }
    }
}

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
    cxxopts::Options options("Camera-capture", "A demo for camera-capture – lightweight, easily adjustable library for "
                                               "managing v4l cameras and capturing frames.");

    options.add_options()
        ("c, camera", "Filename of a camera device", cxxopts::value(config.camera_filename)->default_value("/dev/video0"))
        ("t, type", "Frame type (allowed values: YUYV, JPG, BGRA, AR24, RGGB, RG12)",  cxxopts::value(config.type)) // TODO: more formats
        ("o, out", "Path to save the frame", cxxopts::value(config.out_filename))
        ("d, dims", "Frame width and height (eg. `960,720`)", cxxopts::value(config.dims))
        ("h, help", "Print usage");

    std::vector<std::string> required = {"type", "dims"};
    std::unordered_map<std::string, unsigned int> pix_formats = {// TODO: more formats
                                                                 {"YUYV", V4L2_PIX_FMT_YYUV},
                                                                 {"JPG", V4L2_PIX_FMT_MJPEG},
                                                                 {"BGRA", V4L2_PIX_FMT_ABGR32 },
                                                                 {"AR24", V4L2_PIX_FMT_ABGR32},
                                                                 {"RGGB", V4L2_PIX_FMT_SRGGB8 },
                                                                 {"RG10", V4L2_PIX_FMT_SRGGB10},
                                                                 {"RG12", V4L2_PIX_FMT_SRGGB12}};

    // Get command line parameters and parse them
    try
    {
        result = options.parse(argc, argv);
        checkRequiredArgs(result, required);
        config.pix_format = pix_formats.at(config.type);
    }
    catch (cxxopts::OptionException e)
    {
        std::cerr << std::endl
                  << "\033[31mError while parsing command line arguments: " << e.what() << "\033[0m" << std::endl
                  << std::endl;
        std::cout << options.help() << std::endl;
        exit(1);
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
    catch (CameraException e)
    {
        std::cerr << "\033[31m" << e.what() << "\033[0m" << std::endl << std::endl;
        std::cout << options.help() << std::endl;
        exit(1);
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

static void enumerateMenu(int fd)
{

    struct v4l2_querymenu querymenu;
    printf("\n  Menu items:\n");

    struct v4l2_queryctrl queryctrl;
    memset(&querymenu, 0, sizeof(querymenu));
    querymenu.id = queryctrl.id;

    for (querymenu.index = queryctrl.minimum; querymenu.index <= queryctrl.maximum; querymenu.index++)
    {
        if (0 == ioctl(fd, VIDIOC_QUERYMENU, &querymenu))
        {
            printf("  %s\n", querymenu.name);
        }
    }
}

// for debugging
/**
 * Show all parameters for the camera and set them to default values
 */
void printAllCameraParams(CameraCapture &camera)
{
    int value;
    std::cout << "\nCONTROLS\n";

    struct v4l2_queryctrl queryctrl;
    memset(&queryctrl, 0, sizeof(queryctrl));

    for (queryctrl.id = V4L2_CID_BASE; queryctrl.id < V4L2_CID_LASTP1; queryctrl.id++)
    {
        if (0 == ioctl(camera.getFd(), VIDIOC_QUERYCTRL, &queryctrl))
        {
            if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
            {
                continue;
            }

            try
            {
                std::cout << "Control " << queryctrl.id << " " << queryctrl.name << " ";
                camera.get(queryctrl.id, value);
                std::cout << value << " default: ";
                camera.get(queryctrl.id, value, false);

                // set them to default value
                std::cout << value << std::endl;
                camera.set(queryctrl.id, value);
            }
            catch (CameraException e)
            {
                std::cout << "\e[31m" << e.what() << "\e[0m" << std::endl;
            }

            if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
            {
                enumerateMenu(camera.getFd());
            }
        }
        else
        {
            if (errno == EINVAL)
            {
                continue;
            }

            perror("VIDIOC_QUERYCTRL");
            exit(EXIT_FAILURE);
        }
    }

    std::cout << "Private Base: \n";
    for (queryctrl.id = V4L2_CID_PRIVATE_BASE;; queryctrl.id++)
    {
        if (0 == ioctl(camera.getFd(), VIDIOC_QUERYCTRL, &queryctrl))
        {
            if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
            {
                continue;
            }

            printf("Control %s\n", queryctrl.name);

            if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
            {
                enumerateMenu(camera.getFd());
            }
        }
        else
        {
            if (errno == EINVAL)
            {
                break;
            }

            perror("VIDIOC_QUERYCTRL");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char const *argv[])
{
    // SET UP THE CAMERA
    bool raw = false;
    int input_format;

    Config conf = parseOptions(argc, argv);     ///< user's configuration
    CameraCapture camera(conf.camera_filename); ///< cameracapture object

    // adjust camera settings
    camera.setFormat(conf.dims[0], conf.dims[1], conf.pix_format); // set frame format
    auto format = camera.getFormat();                              ///< Actually set frame format
    double time_perframe;

    ///////////////////////////////////////////////////////////////////////////////////////////////

    v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    camera.runIoctl(VIDIOC_G_FMT, &fmt);

    std::cout << "Format set to " << conf.type << " " << fmt.fmt.pix.pixelformat << ", " << std::get<0>(format) << " x "
              << std::get<1>(format) << std::endl;

    // CAPTURE FRAME
    if (conf.out_filename != "")
    {
        setConverter(camera, conf.pix_format, raw, input_format);

        if (!raw)
        {
            cv::Mat processed_frame = camera.capture(input_format); ///< captured frame
            saveToFile(conf.out_filename + ".png", processed_frame);                        // save it
        }
        else
        {
            std::shared_ptr<MMapBuffer> raw_frame;                     ///< Frame fetched from the camera
            camera.grab();                                             // fetch the frame to camera's buffer 0
            camera.read(raw_frame);                                    // read content from the buffer
            rawToFile(conf.out_filename + "." + conf.type, raw_frame); // save it
        }
    }
    return 0;
}
