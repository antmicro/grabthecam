#include "grabthecam/cameracapture.hpp"
#include "grabthecam/cameracapturetemplates.hpp"
#include "grabthecam/pixelformatsinfo.hpp"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

#include <fcntl.h> // O_RDWR
#include <fstream> //save config
#include <iostream>
#include <libv4l2.h>
#include <sstream>
#include <sys/ioctl.h> // ioctl
#include <vector>

namespace grabthecam
{

#define CAMERA_CLASS_CONTROLS_END V4L2_CID_CAMERA_CLASS_BASE + 36

int xioctl(int fd, int request, void *arg)
{
    int res;
    do
    {
        res = ioctl(fd, request, arg);
    } while (-1 == res && EINTR == errno); // A signal was caught
    return res;
}

CameraCapture::CameraCapture(std::string filename) : converter(nullptr)
{
    // Open the device
    fd = v4l2_open(filename.c_str(), O_RDWR);

    if (fd < 0)
    {
        throw CameraException("Failed to open the camera");
    }

    ready_to_capture = false;
    updateFormat();
}

CameraCapture::~CameraCapture()
{
    // end streaming
    runIoctl(VIDIOC_STREAMOFF, &buffer_type);
    v4l2_close(fd);
}

void CameraCapture::stopStreaming()
{
    if (ready_to_capture)
    {
        // stop streaming
        int type = buffer_type;
        if (xioctl(fd, VIDIOC_STREAMOFF, &type) < 0)
        {
            throw CameraException("Could not end streaming. See errno and VIDEOC_STREAMOFF docs for more information");
        }

        // free buffers
        requestBuffers(0);
        buffers.clear();
        ready_to_capture = false;
    }
}

void CameraCapture::setFormat(unsigned int width, unsigned int height, unsigned int pixelformat, bool keep_converter)
{
    stopStreaming();

    // Set Image format
    v4l2_format fmt = {0};

    fmt.type = buffer_type;
    if (!((width == 0) && (height == 0)))
    {
        fmt.fmt.pix.width = width;
        fmt.fmt.pix.height = height;
    }
    fmt.fmt.pix.pixelformat = pixelformat;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    if (xioctl(fd, VIDIOC_S_FMT, &fmt) < 0)
    {
        throw CameraException("Setting format failed. See errno and VIDEOC_S_FMT docs for more information");
    }
    else
    {
        updateFormat(keep_converter);
    }
}

void CameraCapture::autoSetConverter()
{
    v4l2_format fmt = {0};
    fmt.type = buffer_type;
    if (xioctl(fd, VIDIOC_G_FMT, &fmt) < 0)
    {
        throw CameraException("Getting format failed. See errno and VIDEOC_G_FMT docs for more information");
    }

    unsigned int pixelformat = fmt.fmt.pix.pixelformat;
    try
    {
        setConverter(formats_info.at(pixelformat)());
    }
    catch (std::out_of_range)
    {
        std::cout << "Type " << pixelformat << " unrecognised. Set converter manually if needed.\n";
        setConverter(nullptr);
    }
}

void CameraCapture::updateFormat(bool keep_converter)
{
    v4l2_format fmt = {0};
    fmt.type = buffer_type;

    if (xioctl(fd, VIDIOC_G_FMT, &fmt) < 0)
    {
        throw CameraException("Getting format failed. See errno and VIDEOC_G_FMT docs for more information");
    }

    height = fmt.fmt.pix.height;
    width = fmt.fmt.pix.width;
    if (!keep_converter)
    {
        autoSetConverter();
    }
    this->v4l2_format_code = fmt.fmt.pix.pixelformat;
}

void CameraCapture::runIoctl(int ioctl, void *value) const
{
    if (v4l2_ioctl(fd, ioctl, value) != 0)
    {
        throw CameraException("runIoctl: v4l2_ioctl error [ioctl: " + std::to_string(ioctl) + "]", errno);
    }
}

void CameraCapture::queryProperty(int property, v4l2_queryctrl &query) const
{
    memset(&query, 0, sizeof(query));
    query.id = property;

    if (xioctl(fd, VIDIOC_QUERYCTRL, &query) == -1)
    {
        if (errno != EINVAL)
        {
            throw CameraException("queryProperty: vidioc_queryctrl error", errno);
        }
        else
        {
            throw CameraException("queryProperty: vidioc_queryctrl error: Property is not supported.", errno);
        }
    }
    else if (query.flags & V4L2_CTRL_FLAG_DISABLED)
    {
        throw CameraException("queryProperty: vidioc_queryctrl error: Property is disabled.", errno);
    }
}

void CameraCapture::getCtrls(int property, bool current, v4l2_ext_controls &ctrls) const
{
    v4l2_queryctrl queryctrl;

    queryProperty(property, queryctrl); // can throw exception
    v4l2_ext_control ctrl[1];
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl[0].id = property;
    ctrl[0].size = 0;

    memset(&ctrls, 0, sizeof(ctrls));
    ctrls.which = current ? V4L2_CTRL_WHICH_CUR_VAL : V4L2_CTRL_WHICH_DEF_VAL;
    ctrls.count = 1;
    ctrls.controls = ctrl;

    try
    {
        runIoctl(VIDIOC_G_EXT_CTRLS, &ctrls);
    }
    catch (CameraException e)
    {
        switch (e.error_code)
        {
        case EINVAL:
            throw CameraException("Check if your stucture is valid and you've filled all required fields.",
                                  e.error_code);
            break;
        case ENOSPC:
            throw CameraException("Too small size was set. Changed to " + std::to_string(ctrl[0].size), e.error_code);
            break;
        default:
            throw CameraException("", e.error_code);
        }
    }
}

void CameraCapture::setCtrl(int property, v4l2_ext_control *ctrl, bool warning)
{
    int res;
    int value = ctrl[0].value;

    ctrl[0].id = property;
    ctrl[0].size = 0;

    v4l2_queryctrl queryctrl;
    queryProperty(property, queryctrl); // can throw exception

    v4l2_ext_controls ctrls;
    memset(&ctrls, 0, sizeof(ctrls));
    ctrls.which = V4L2_CTRL_WHICH_CUR_VAL;
    ctrls.count = 1;
    ctrls.controls = ctrl;

    try
    {
        runIoctl(VIDIOC_S_EXT_CTRLS, &ctrls);
    }
    catch (CameraException e)
    {
        switch (e.error_code)
        {
        case EINVAL:
            throw CameraException("Check if your stucture is valid and you've filled all required fields.",
                                  e.error_code);
            break;
        case ERANGE:
            throw CameraException("Wrong parameter value. It should be between " + std::to_string(queryctrl.minimum) +
                                  " and " + std::to_string(queryctrl.maximum) +
                                  " (step: " + std::to_string(queryctrl.step) + ")");
            break;
        case EILSEQ:
            throw CameraException("Check if your change is compatible with other camera settings.", e.error_code);
            break;
        default:
            throw CameraException("", e.error_code);
        }
    }

    if (warning && value != ctrl[0].value)
    {
        std::cerr << "\n[WARNING] Parameter's value was clamped to " << ctrl[0].value
                  << ". It should be between " + std::to_string(queryctrl.minimum) + " and " +
                         std::to_string(queryctrl.maximum) + " (step: " + std::to_string(queryctrl.step) + ")\n";
    }
}

void CameraCapture::requestBuffers(int n, std::vector<void *> locations)
{
    if (locations.size() == 0)
    {
        for (int i = 0; i < n; i++)
        {
            locations.push_back(NULL);
        }
    }
    else if (locations.size() != n)
    {
        throw CameraException("Invalid locations lenght. It should be equal to n");
    }

    buffers.clear();

    // Request FrameBufferInfo from the device, which will be used for capturing frames
    struct v4l2_requestbuffers request_buffer = {0};
    request_buffer.count = n;
    request_buffer.type = buffer_type;
    request_buffer.memory = V4L2_MEMORY_MMAP;

    if (xioctl(fd, VIDIOC_REQBUFS, &request_buffer) < 0)
    {
        throw CameraException("Requesting buffer failed. See errno and VIDEOC_REQBUFS docs for more information.");
    }

    // ask for the requested buffers

    struct v4l2_buffer query_buffer;
    std::shared_ptr<char> start;

    for (int i = 0; i < request_buffer.count; i++)
    {
        memset(&query_buffer, 0, sizeof(query_buffer));

        query_buffer.type = buffer_type;
        query_buffer.memory = V4L2_MEMORY_MMAP;
        query_buffer.index = i;

        if (xioctl(fd, VIDIOC_QUERYBUF, &query_buffer) < 0)
        {
            throw CameraException("Device did not return the queryBuffer information. See errno and VIDEOC_QUERYBUF "
                                  "docs for more information.");
        }

        // use a pointer to point to the newly created queryBuffer
        // map the memory address of the device to an address in memory
        buffers.push_back(std::make_shared<MMapBuffer>(locations[i], query_buffer.length, fd, query_buffer.m.offset));
    }
}

std::pair<int, int> CameraCapture::getFormat() const { return std::pair<int, int>(width, height); }

void CameraCapture::enumerateMenu(v4l2_queryctrl &queryctrl) const
{
    struct v4l2_querymenu querymenu;
    printf("        Available items:\n");

    memset(&querymenu, 0, sizeof(querymenu));
    querymenu.id = queryctrl.id;

    for (querymenu.index = queryctrl.minimum; querymenu.index <= queryctrl.maximum; querymenu.index++)
    {
        if (0 == xioctl(fd, VIDIOC_QUERYMENU, &querymenu))
        {
            std::cout << "        " << querymenu.index << ". " << querymenu.name << std::endl;
        }
    }
}

int CameraCapture::printControl(v4l2_queryctrl &queryctrl) const
{
    if (0 == xioctl(fd, VIDIOC_QUERYCTRL, &queryctrl))
    {
        try
        {
            int value;
            get(queryctrl.id, value);
            std::cout << queryctrl.id << " " << queryctrl.name << ": " << value << " (default: ";
            get(queryctrl.id, value, false);
            std::cout << value << ")" << std::endl;
        }
        catch (CameraException e)
        {
            std::cout << "\e[31m" << e.what() << "\e[0m" << std::endl;
        }

        if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
        {
            enumerateMenu(queryctrl);
        }
    }
    else
    {
        if (errno == EINVAL)
        {
            return 1;
        }
    }
    return 0;
}

void CameraCapture::printControls() const
{
    int value;
    std::cout << "\nCONTROLS\n"
              << "------------------\n";

    struct v4l2_queryctrl queryctrl;
    memset(&queryctrl, 0, sizeof(queryctrl));

    for (queryctrl.id = V4L2_CID_BASE; queryctrl.id < V4L2_CID_LASTP1; queryctrl.id++)
    {
        printControl(queryctrl);
    }

    // private base
    for (queryctrl.id = V4L2_CID_PRIVATE_BASE;; queryctrl.id++)
    {
        if (printControl(queryctrl) == 1)
        {
            break;
        }
    }
    for (queryctrl.id = V4L2_CID_CAMERA_CLASS_BASE; queryctrl.id < CAMERA_CLASS_CONTROLS_END; queryctrl.id++)
    {
        printControl(queryctrl);
    }

    std::cout << std::endl;
}

void CameraCapture::grab(int buffer_no, int number_of_buffers, std::vector<void *> locations)
{
    if (ready_to_capture && buffers.size() != number_of_buffers)
    {
        stopStreaming();
    }

    if (!ready_to_capture)
    {
        requestBuffers(number_of_buffers, locations); // buffers in the device memory

        info_buffer = std::make_shared<v4l2_buffer>();
        memset(info_buffer.get(), 0, sizeof(info_buffer));
        info_buffer->type = buffer_type;
        info_buffer->memory = V4L2_MEMORY_MMAP;

        // Activate streaming
        if (xioctl(fd, VIDIOC_STREAMON, &buffer_type) < 0)
        {
            throw CameraException(
                "Could not start streaming. See errno and VIDEOC_STREAMON docs for more information.");
        }

        ready_to_capture = true;
    }

    info_buffer->index = buffer_no;

    // Queue the buffer
    if (xioctl(fd, VIDIOC_QBUF, info_buffer.get()) < 0)
    {
        throw CameraException("Could not queue the buffer. See errno and VIDEOC_QBUF docs for more information.");
    }

    // Dequeue the buffer
    if (xioctl(fd, VIDIOC_DQBUF, info_buffer.get()) < 0)
    {
        throw CameraException("Could not dequeue the buffer. See errno and VIDEOC_DQBUF docs for more information.");
    }
    // Frames get written after dequeuing the buffer
    buffers[buffer_no].get()->bytesused = info_buffer->bytesused;
}

void CameraCapture::checkBuffer(int buffer_no) const
{
    if (!(buffers.size() > buffer_no && buffers[buffer_no]->bytesused > 0))
    {
        throw CameraException("Cannot read the frame from the buffer " + std::to_string(buffer_no) +
                              ". Grab the frame first.");
    }
}

void CameraCapture::read(std::shared_ptr<MMapBuffer> &frame, int buffer_no) const
{
    checkBuffer(buffer_no);
    frame = buffers[buffer_no];
}

void CameraCapture::read(std::shared_ptr<cv::Mat> &frame, int dtype, int buffer_no) const
{
    checkBuffer(buffer_no);

    // Some pixel formats with chroma subsampling require the rescaling of the buffer
    // to properly interface with OpenCV

    switch (this->v4l2_format_code)
    {
    case V4L2_PIX_FMT_YUV422P:
    case V4L2_PIX_FMT_YVU420:
    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_NV21:
    case V4L2_PIX_FMT_NV12:
    {
        frame = std::make_shared<cv::Mat>(cv::Mat(height * 3 / 2, width, dtype, buffers[buffer_no]->start));
        break;
    }
    default:
        frame = std::make_shared<cv::Mat>(cv::Mat(height, width, dtype, buffers[buffer_no]->start));
    }
}

void CameraCapture::read(cv::Mat &frame, int dtype, int buffer_no) const
{
    checkBuffer(buffer_no);
    frame = cv::Mat(height, width, dtype, buffers[buffer_no]->start);
}

cv::Mat CameraCapture::capture(int raw_frame_dtype, int buffer_no, int number_of_buffers, std::vector<void *> locations)
{
    // set raw_frame_dtype from converter
    if (hasConverter())
    {
        if (raw_frame_dtype != -1 && raw_frame_dtype != converter->input_format)
        {
            throw CameraException(
                "capture: raw_frame_dtype shouldn't be provided for the cameracapture with a converter");
        }
        else
        {
            raw_frame_dtype = converter->input_format;
        }
    }

    std::shared_ptr<cv::Mat> frame;

    grab(buffer_no, number_of_buffers, locations);
    read(frame, raw_frame_dtype, buffer_no);

    if (hasConverter())
    {
        frame = std::make_shared<cv::Mat>(converter->convert(*frame));
    }
    else
    {
        std::cerr << "WARNING: No converter provided - ommiting preprocessing\n";
    }
    return *frame;
}

std::string CameraCapture::getConfigFilename()
{
    // get the driver name
    v4l2_capability cap;
    try
    {
        runIoctl(VIDIOC_QUERYCAP, &cap);
    }
    catch (CameraException)
    {
        return ".pyvidctrl-unknown-driver";
    }

    std::stringstream ss;
    ss << ".pyvidctrl-" << cap.driver;
    return ss.str();
}

void CameraCapture::saveTriggerInfo(TriggerInfo trigger_info, rapidjson::PrettyWriter<rapidjson::StringBuffer> &writer)
{
    writer.StartObject();
    writer.Key("extraopts");
    writer.String("trigger_info");
    writer.Key("id");
    writer.Int(0);
    writer.Key("name");
    writer.String("trigger_info");
    writer.Key("trig_offset");
    writer.Int(trigger_info.mode_reg);
    writer.Key("src_offset");
    writer.Int(trigger_info.source_reg);
    writer.Key("activation_offset");
    writer.Int(trigger_info.activation_reg);
    writer.Key("src_value");
    writer.Int(trigger_info.source_value);
    writer.Key("activation_value");
    writer.Int(trigger_info.activation_mode);
    writer.EndObject();
}

std::string CameraCapture::saveConfig(std::string filename)
{
    rapidjson::StringBuffer s;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);

    struct v4l2_queryctrl queryctrl;
    memset(&queryctrl, 0, sizeof(queryctrl));

    // Get values for all controls

    writer.StartArray();
    for (queryctrl.id = V4L2_CID_BASE; queryctrl.id < V4L2_CID_LASTP1; queryctrl.id++)
    {
        saveControlValue(queryctrl, writer);
    }

    for (queryctrl.id = V4L2_CID_PRIVATE_BASE;; queryctrl.id++)
    {
        if (saveControlValue(queryctrl, writer) == 1)
        {
            break;
        };
    }
    for (queryctrl.id = V4L2_CID_CAMERA_CLASS_BASE; queryctrl.id < CAMERA_CLASS_CONTROLS_END; queryctrl.id++)
    {
        saveControlValue(queryctrl, writer);
    }

    if (this->trigger_info.has_value())
    {
        saveTriggerInfo(this->trigger_info.value(), writer);
    }
    writer.EndArray();

    std::fstream file;

    if (filename == "")
    {
        filename = getConfigFilename();
    }
    createDirectories(filename);
    file.open(filename, std::fstream::out);
    if (!file.is_open())
    {
        throw CameraException("Save configuration: Could not open file for writing");
    }
    file << s.GetString();
    file.close();

    return filename;
}

std::string CameraCapture::loadConfig(std::string filename)
{
    // Read the file
    if (filename == "")
    {
        filename = getConfigFilename();
    }
    std::ifstream file{filename};
    if (!file.is_open())
    {
        throw CameraException("Load configuration: Could not open file for reading");
    }

    rapidjson::IStreamWrapper wrapper{file};

    rapidjson::Document doc;
    doc.ParseStream(wrapper);

    if (doc.HasParseError())
    {
        throw CameraException("Wrong configuration file format: " +
                              std::string(rapidjson::GetParseError_En(doc.GetParseError())));
    }

    auto properties = doc.GetArray();
    int id, value;

    for (auto itr = properties.Begin(); itr != properties.End(); itr++)
    {
        const rapidjson::Value &property = *itr;
        if (!property.IsObject())
        {
            throw CameraException("Wrong configuration file format: The property is not an object.");
        }

        if (property.HasMember("extraopts"))
        {
            if ((std::string)property.FindMember("extraopts")->value.GetString() == "trigger_info")
            {
                TriggerInfo info;
                info.mode_reg = property.FindMember("trig_offset")->value.GetInt();
                info.source_reg = property.FindMember("src_offset")->value.GetInt();
                info.source_value = property.FindMember("src_value")->value.GetInt();
                info.activation_reg = property.FindMember("activation_offset")->value.GetInt();
                info.activation_mode = property.FindMember("activation_value")->value.GetInt();
                this->trigger_info.emplace(info);
            }
            return filename;
        }
        // Apply the values
        id = itr->FindMember("id")->value.GetInt();
        value = itr->FindMember("value")->value.GetInt();

        try
        {
            set(id, value);
        }
        catch (CameraException e)
        {
            std::cerr << "[WARNING] Cannot set " << itr->FindMember("name")->value.GetString() << " (Error " << e.what()
                      << ")\n";
        }
    }

    return filename;
}

int CameraCapture::saveControlValue(v4l2_queryctrl &queryctrl, rapidjson::PrettyWriter<rapidjson::StringBuffer> &writer)
{
    int value;
    if (xioctl(fd, VIDIOC_QUERYCTRL, &queryctrl) == 0)
    {
        if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
        {
            return 0;
        }

        get(queryctrl.id, value);
        writer.StartObject();
        writer.Key("id");
        writer.Int(queryctrl.id);
        writer.Key("name");
        writer.String(reinterpret_cast<char const *>(queryctrl.name));
        writer.Key("type");
        writer.Int(queryctrl.type);
        writer.Key("value");
        writer.Int(value);
        writer.EndObject();
    }
    else
    {
        if (errno == EINVAL)
        {
            return 1;
        }
    }
    return 0;
}

CameraCapture::CameraPropertyStatus CameraCapture::queryProperty(int32_t propertyID, CameraProperty &property) const
{
    v4l2_queryctrl queryctrl;
    memset(&queryctrl, 0, sizeof(queryctrl));
    queryctrl.id = propertyID;

    if (!xioctl(fd, VIDIOC_QUERYCTRL, &queryctrl))
    {
        if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
        {
            return CameraPropertyStatus::DISABLED;
        }
        get(queryctrl.id, property.defaultValue, false);
        property.id = propertyID;
        property.name = (char *)queryctrl.name;
        property.type = queryctrl.type;

        return CameraPropertyStatus::ENABLED;
    }
    else if (errno == EINVAL)
    {
        return CameraPropertyStatus::UNSUPPORTED;
    }
    else
    {
        throw CameraException("Unknown error");
    }
}

std::vector<CameraCapture::CameraProperty> CameraCapture::queryProperties() const
{
    std::vector<CameraProperty> result;
    CameraProperty property;

    std::vector<int32_t> properties;
    for (int i = V4L2_CID_BASE; i < V4L2_CID_LASTP1; i++)
    {
        properties.push_back(i);
    }
    for (int i = V4L2_CID_CAMERA_CLASS_BASE; i < V4L2_CID_CAMERA_CLASS_BASE + 36; i++)
    {
        properties.push_back(i);
    }

    for (int32_t property_id : properties)
    {
        if (queryProperty(property_id, property) != CameraPropertyStatus::ENABLED)
        {
            continue;
        }

        result.push_back(property);
    }
    for (int property_id = V4L2_CID_PRIVATE_BASE;; property_id++)
    {
        CameraPropertyStatus status = queryProperty(property_id, property);

        if (status == CameraPropertyStatus::DISABLED)
        {
            continue;
        }

        if (status == CameraPropertyStatus::UNSUPPORTED)
        {
            break;
        }

        result.push_back(property);
    }

    return result;
}

std::vector<CameraCapture::CameraPropertyMenuEntry> CameraCapture::queryPropertyMenuEntries(int32_t propertyID) const
{
    std::vector<CameraCapture::CameraPropertyMenuEntry> result;

    v4l2_queryctrl queryctrl;
    memset(&queryctrl, 0, sizeof(queryctrl));
    queryctrl.id = propertyID;
    xioctl(fd, VIDIOC_QUERYCTRL, &queryctrl);

    v4l2_querymenu querymenu;
    memset(&querymenu, 0, sizeof(querymenu));
    querymenu.id = queryctrl.id;

    for (querymenu.index = queryctrl.minimum; querymenu.index <= queryctrl.maximum; querymenu.index++)
    {
        if (not xioctl(fd, VIDIOC_QUERYMENU, &querymenu))
        {
            result.push_back({querymenu.index, (char *)querymenu.name});
        }
    }

    return result;
}

CameraCapture::CameraPropertyDetails CameraCapture::queryPropertyDetails(int32_t propertyID) const
{
    CameraProperty property;
    if (queryProperty(propertyID, property) != CameraPropertyStatus::ENABLED)
    {
        throw CameraException("Queried against unsupported property");
    }

    return CameraPropertyDetails{.property = property,
                                 .menuEntries = property.type == V4L2_CTRL_TYPE_MENU
                                                    ? queryPropertyMenuEntries(propertyID)
                                                    : std::vector<CameraPropertyMenuEntry>()};
}

void CameraCapture::defaultEnableTrigger() const
{
    TriggerInfo trigger_info = this->trigger_info.value();
    struct v4l2_control enable_trigger = {.id = trigger_info.mode_reg, .value = 1};
    if (ioctl(this->fd, VIDIOC_S_CTRL, &enable_trigger) == -1)
    {
        throw CameraException("Error while enabling trigger ");
    }

    struct v4l2_control set_trigger_source = {.id = trigger_info.source_reg, .value = trigger_info.source_value};
    if (ioctl(this->fd, VIDIOC_S_CTRL, &set_trigger_source) == -1)
    {
        throw CameraException("Error while setting trigger source");
    }

    struct v4l2_control set_trigger_activation = {.id = trigger_info.activation_reg,
                                                  .value = trigger_info.activation_mode};
    if (ioctl(this->fd, VIDIOC_S_CTRL, &set_trigger_activation) == -1)
    {
        throw CameraException("Error while setting trigger activation mode");
    }
}

}; // namespace grabthecam
