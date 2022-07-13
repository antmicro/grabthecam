#include "Camera.hpp"

int xioctl(int fd, int request, void *arg)
{
    int res;
    do
    {
        res = ioctl(fd, request, arg);
    } while (-1 == res && EINTR == errno); //A signal was caught

    return res;
}


Camera::Camera(std::string filename)
{
    //Open the device

    this->fd = v4l2_open(filename.c_str(), O_RDWR | O_CREAT);

    if(fd < 0)
    {
        throw CameraException("Failed to open the camera");
    }
    ready_to_capture = false;
}

Camera::~Camera()
{
    std::cout << "closing...\n";

    // end streaming
    xioctl(fd, VIDIOC_STREAMOFF, &buffer_type);
    v4l2_close(this->fd);
}

int Camera::getCapabilities(ucap_ptr &cap)
{
    // Ask the device if it can capture frames
    if (xioctl(this->fd, VIDIOC_QUERYCAP, cap.get()) < 0)
    {
        throw CameraException("Error in VIDIOC_QUERYCAP. See errno for more information");
        return -1;
    }
    return 0;
}

int Camera::set(int property, double value)
{
    v4l2_control c;
    c.id = property;
    c.value = value;
    if(v4l2_ioctl(this->fd, VIDIOC_S_CTRL, &c) != 0)
    {
        throw CameraException("Setting property failed. See errno and VIDEOC_S_CTRL docs for more information");
        return -1;
    }
    return 0;
}

int Camera::get(int property, double &value)
{
    v4l2_control c;
    c.id = property;
    if(v4l2_ioctl(this->fd, VIDIOC_G_CTRL, &c) != 0)
    {
        throw CameraException("Getting property failed. See errno and VIDEOC_G_CTRL docs for more information");
        return -1;
    }
    value = c.value;
    return 0;
}

int Camera::stopStreaming()
{
    if (ready_to_capture)
    {
        // stop streaming
        int type = buffer_type;
        if(xioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
            throw CameraException("Could not end streaming. See errno and VIDEOC_STREAMOFF docs for more information");
            return -1;
        }

        // free buffers
        //TODO: request 0 buffers

        struct v4l2_requestbuffers requestBuffer = {0};
        requestBuffer.count = 0;
        requestBuffer.type = buffer_type;
        requestBuffer.memory = V4L2_MEMORY_MMAP;

        if (xioctl(this->fd, VIDIOC_REQBUFS, &requestBuffer) < 0)
        {
            throw CameraException("Emptying buffers failed. See errno and VIDEOC_REQBUFS docs for more information");
            return -1;
        }

        buffers.clear();
        ready_to_capture = false;
    }
    return 0;
}

int Camera::setFormat(unsigned int width, unsigned int height, unsigned int pixelformat)
{
    stopStreaming();

    //Set Image format
    v4l2_format fmt = {0};

    fmt.type = buffer_type;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = pixelformat;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    if (xioctl(this->fd, VIDIOC_S_FMT, &fmt) < 0)
    {
        throw CameraException("Setting format failed. See errno and VIDEOC_S_FMT docs for more information");
        return -1;
    }
    else
    {
        updateFormat();
        std::cout << "Format set to " << this->height << "x" << this->width << std::endl;
    }

    return 0;
}

int Camera::updateFormat()
{
    v4l2_format fmt = {0};
    fmt.type = buffer_type;

    if (xioctl(this->fd, VIDIOC_G_FMT, &fmt) < 0)
    {
        throw CameraException("Getting format failed. See errno and VIDEOC_G_FMT docs for more information");
        return -1;
    }

    this->height = fmt.fmt.pix.height;
    this->width = fmt.fmt.pix.width;

    return 0;
}


int Camera::requestBuffers(int n, std::vector<void*> locations)
{
    if (locations.size() == 0)
    {
        for (int i=0 ; i < n; i++)
        {
            locations.push_back(NULL);
        }
    }
    else if (locations.size() != n)
    {
        throw CameraException("Invalid locations lenght. It should be equal to n");
        return -1;
    }

    buffers.clear();

    // Request FrameBufferInfo from the device, which will be used for capturing frames
    struct v4l2_requestbuffers requestBuffer = {0};
    requestBuffer.count = n;
    requestBuffer.type = buffer_type;
    requestBuffer.memory = V4L2_MEMORY_MMAP;

    if (xioctl(this->fd, VIDIOC_REQBUFS, &requestBuffer) < 0)
    {
        throw CameraException("Requesting buffer failed. See errno and VIDEOC_REQBUFS docs for more information.");
        return -1;
    }

    std::cout << "Reqbuf " << requestBuffer.count << std::endl; //if < 2 insufficient?

    // ask for the requested buffers

    struct v4l2_buffer queryBuffer;
    schar_ptr start;

    for (int i=0; i < requestBuffer.count; i++)
    {
	    memset (&queryBuffer, 0, sizeof(queryBuffer));

        queryBuffer.type = buffer_type;
        queryBuffer.memory = V4L2_MEMORY_MMAP;
        queryBuffer.index = i;

        if(xioctl(this->fd, VIDIOC_QUERYBUF, &queryBuffer) < 0)
        {
            throw CameraException("Device did not return the queryBuffer information. See errno and VIDEOC_QUERYBUF docs for more information.");
            return -1;
        }

        // use a pointer to point to the newly created queryBuffer
        // map the memory address of the device to an address in memory
        buffers.push_back(std::make_shared<FrameBufferInfo>(
            locations[i], queryBuffer.length, fd, queryBuffer.m.offset
        ));
    }
    return 0;
}

int Camera::capture(uframe_ptr &frame, int buffer_no, std::vector<void*> locations)
{
    return capture(frame, buffer_no, locations.size(), locations);
}

int Camera::capture(uframe_ptr &frame, int buffer_no, int number_of_buffers, std::vector<void*> locations)
{
    std::cout << "Capture\n";
    //TODO: test
    if (ready_to_capture && buffers.size() != number_of_buffers)
    {
        stopStreaming();
    }

    if (!ready_to_capture)
    {
        std::cout << "Preparing to capture...\n";
        requestBuffers(number_of_buffers, locations); // buffers in the device memory

        info_buffer = std::make_shared<v4l2_buffer>();
        memset(info_buffer.get(), 0, sizeof(info_buffer));
        info_buffer->type = buffer_type;
        info_buffer->memory = V4L2_MEMORY_MMAP;

        // Activate streaming
        if(xioctl(this->fd, VIDIOC_STREAMON, &buffer_type) < 0)
        {
            throw CameraException("Could not start streaming. See errno and VIDEOC_STREAMON docs for more information.");
            return -1;
        }

        ready_to_capture = true;
    }

    info_buffer->index = buffer_no;

    // Queue the buffer
    if(xioctl(fd, VIDIOC_QBUF, info_buffer.get()) < 0)
    {
        throw CameraException("Could not queue the buffer. See errno and VIDEOC_QBUF docs for more information.");
        return -2;
    }

    // Dequeue the buffer
    if(xioctl(fd, VIDIOC_DQBUF, info_buffer.get()) < 0)
    {
        throw CameraException("Could not dequeue the buffer. See errno and VIDEOC_DQBUF docs for more information.");
        return -2;
    }

    buffers[buffer_no].get() -> bytesused = info_buffer->bytesused;


    // Frames get written after dequeuing the buffer
    frame->assignFrame(buffers[buffer_no], this->width, this->height);

    return 0;
}
