#include "camera-capture/cameracapture.hpp"

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
    this->fd = v4l2_open(filename.c_str(), O_RDWR);

    if (fd < 0)
    {
        throw CameraException("Failed to open the camera");
    }
    ready_to_capture = false;
}

CameraCapture::~CameraCapture()
{
    // end streaming
    xioctl(fd, VIDIOC_STREAMOFF, &buffer_type);
    v4l2_close(this->fd);
}

void CameraCapture::getCapabilities(std::unique_ptr<v4l2_capability> &cap) const
{
    // Ask the device if it can capture frames
    if (xioctl(this->fd, VIDIOC_QUERYCAP, cap.get()) < 0)
    {
        throw CameraException("Error in VIDIOC_QUERYCAP. See errno for more information");
    }
}

void CameraCapture::set(int property, double value)
{
    v4l2_control c;
    c.id = property;
    c.value = value;
    if (v4l2_ioctl(this->fd, VIDIOC_S_CTRL, &c) != 0)
    {
        throw CameraException("Setting property failed. See errno and VIDEOC_S_CTRL docs for more information");
    }
}

void CameraCapture::get(int property, double &value) const
{
    v4l2_control c;
    c.id = property;
    if (v4l2_ioctl(this->fd, VIDIOC_G_CTRL, &c) != 0)
    {
        throw CameraException("Getting property failed. See errno and VIDEOC_G_CTRL docs for more information");
    }
    value = c.value;
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

void CameraCapture::setFormat(unsigned int width, unsigned int height, unsigned int pixelformat)
{
    stopStreaming();

    // Set Image format
    v4l2_format fmt = {0};

    fmt.type = buffer_type;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = pixelformat;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    if (xioctl(this->fd, VIDIOC_S_FMT, &fmt) < 0)
    {
        throw CameraException("Setting format failed. See errno and VIDEOC_S_FMT docs for more information");
    }
    else
    {
        updateFormat();
    }
}

void CameraCapture::updateFormat()
{
    v4l2_format fmt = {0};
    fmt.type = buffer_type;

    if (xioctl(this->fd, VIDIOC_G_FMT, &fmt) < 0)
    {
        throw CameraException("Getting format failed. See errno and VIDEOC_G_FMT docs for more information");
    }

    this->height = fmt.fmt.pix.height;
    this->width = fmt.fmt.pix.width;
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

    if (xioctl(this->fd, VIDIOC_REQBUFS, &request_buffer) < 0)
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

        if (xioctl(this->fd, VIDIOC_QUERYBUF, &query_buffer) < 0)
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
        if (xioctl(this->fd, VIDIOC_STREAMON, &buffer_type) < 0)
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

void CameraCapture::read(std::shared_ptr<MMapBuffer> &frame, int buffer_no) const { frame = buffers[buffer_no]; }

void CameraCapture::read(std::shared_ptr<cv::Mat> &frame, int dtype, int buffer_no) const
{
    frame = std::make_shared<cv::Mat>(cv::Mat(height, width, dtype, buffers[buffer_no]->start));
}

void CameraCapture::read(cv::Mat *frame, int dtype, int buffer_no) const
{
    frame = new cv::Mat(height, width, dtype, buffers[buffer_no]->start);
}

cv::Mat CameraCapture::capture(int raw_frame_dtype, int buffer_no, int number_of_buffers, std::vector<void *> locations)
{
    std::shared_ptr<cv::Mat> frame;

    grab(buffer_no, number_of_buffers, locations);
    read(frame, raw_frame_dtype, buffer_no);

    if (converter != nullptr)
    {
        frame = std::make_shared<cv::Mat>(converter->convert(*frame));
    }
    else
    {
        std::cerr << "WARNING: No converter provided - ommiting preprocessing\n";
    }
    return *frame;
}
