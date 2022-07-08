#include "../includes/Camera.hpp"

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

    this -> fd = v4l2_open(filename.c_str(), O_RDWR | O_CREAT);
    if(fd < 0)
    {
        std::cerr << "Failed to open the device\n";
    }
    ready_to_capture = false;
}

Camera::~Camera()
{
    std::cout << "closing...\n";

    // end streaming
    int type = info_buffer -> type;
    xioctl(fd, VIDIOC_STREAMOFF, &type);
    v4l2_close(this -> fd);
}

int Camera::getCapabilities(ucap_ptr &cap)
{
    // Ask the device if it can capture frames
    if (xioctl(this -> fd, VIDIOC_QUERYCAP, cap.get()) < 0)
    {
        if (errno == EINVAL)
        {
            std::cerr <<  "This is not a V4L2 device\n";
        }
        else
        {
            std::cerr << "Error in ioctl VIDIOC_QUERYCAP\n";
        }
        return -1;
    }
    return 0;
}

int Camera::set(int property, double value)
{
    v4l2_control c;
    c.id = property;
    c.value = value;
    if(v4l2_ioctl(this -> fd, VIDIOC_S_CTRL, &c) != 0)
    {
        if (errno == EINVAL)
        {
            std::cerr << "Invalid query \"" << property << " = " << value << "\"" << std::endl;
        }
        else
        {
            std::cerr << "Setting property failed errno:" << errno << std::endl;
        }
        return -1;
    }
    return 0;
}

int Camera::get(int property, double &value)
{
    v4l2_control c;
    c.id = property;
    if(v4l2_ioctl(this -> fd, VIDIOC_G_CTRL, &c) != 0)
    {
        if (errno == EINVAL)
        {
            std::cerr << "Not a valid property " << property << std::endl;
        }
        else
        {
            std::cerr << "Getting property failed errno:" << errno << std::endl;
        }
        return -1;
    }
    value = c.value;
    return 0;
}

int Camera::setFormat(unsigned int width, unsigned int height, unsigned int pixelformat)
{
    if (ready_to_capture)
    {
        // stop streaming
        int type = info_buffer -> type;
        if(xioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
            std::cerr << ("Could not end streaming, VIDIOC_STREAMOFF");
            return -1;
        }

        // free buffers
        //TODO: apply request n buffers method
        struct v4l2_requestbuffers requestBuffer = {0};
        requestBuffer.count = 0;
        requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        requestBuffer.memory = V4L2_MEMORY_MMAP;

        if (xioctl(this -> fd, VIDIOC_REQBUFS, &requestBuffer) < 0)
        {
            std::cerr << "Requesting Buffer failed\n";
            return -1;
        }

        frame_buffer = nullptr;
        info_buffer = nullptr;

        std::cout << frame_buffer.use_count() << " " << info_buffer.use_count() << std::endl;

        ready_to_capture = false;
    }

    //Set Image format
    v4l2_format fmt = {0};

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = pixelformat;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    if (xioctl(this -> fd, VIDIOC_S_FMT, &fmt) < 0)
    {
        std::cerr << "VIDIOC_S_FMT failed " << errno <<std::endl;
        return -1;
    }
    else
    {
	    updateFormat();
        std::cout << "Format set to " << this -> height << "x" << this -> width << std::endl;
    }

    return 0;
}

int Camera::updateFormat()
{
    v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (xioctl(this -> fd, VIDIOC_G_FMT, &fmt) < 0)
    {
        std::cerr << "VIDIOC_G_FMT failed" << errno << std::endl;
        return -1;
    }

    this -> height = fmt.fmt.pix.height;
    this -> width = fmt.fmt.pix.width;

    return 0;
}

int Camera::requestBuffer(void *location=NULL)
{
    //TODO: request n buffers

    // Request Buffer from the device, which will be used for capturing frames
    struct v4l2_requestbuffers requestBuffer = {0};
    requestBuffer.count = 1;
    requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    requestBuffer.memory = V4L2_MEMORY_MMAP;

    if (xioctl(this -> fd, VIDIOC_REQBUFS, &requestBuffer) < 0)
    {
        std::cerr << "Requesting Buffer failed\n";
        return -1;
    }

    // ask for the you requested buffer and allocate memory for it
    struct v4l2_buffer queryBuffer = {0};
    queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    queryBuffer.memory = V4L2_MEMORY_MMAP;
    queryBuffer.index = 0;
    if(xioctl(this -> fd, VIDIOC_QUERYBUF, &queryBuffer) < 0)
    {
        std::cerr << "Device did not return the queryBuffer information\n";
        return -2;
    }

    // use a pointer to point to the newly created queryBuffer
    // map the memory address of the device to an address in memory

    char *b = (char*) mmap(location, queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED,
        this -> fd, queryBuffer.m.offset);
    memset(b, 0, queryBuffer.length);

    if (b == (void *) -1)
    {
        std::cerr<<"Mmap failed\n";
        return -3;
    }

    frame_buffer = std::shared_ptr<char>(b, D(queryBuffer.length));

    return 0;
}

int Camera::capture(uframe_ptr &frame, void *location=NULL)
{
    std::cout << "Capture\n";

    if (!ready_to_capture)
    {
        std::cout << "Preparing to capture...\n";
        requestBuffer(location); // buffer in the device memory

        info_buffer = std::make_shared<v4l2_buffer>();
        memset(info_buffer.get(), 0, sizeof(info_buffer));
        info_buffer -> type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        info_buffer -> memory = V4L2_MEMORY_MMAP;
        info_buffer -> index = 0;

        // Activate streaming
        int type = info_buffer -> type;
        if(xioctl(this -> fd, VIDIOC_STREAMON, &type) < 0)
        {
            std::cerr << "Could not start streaming\n";
            return -1;
        }

        ready_to_capture = true;
    }

    // Queue the buffer
    if(xioctl(fd, VIDIOC_QBUF, info_buffer.get()) < 0)
    {
        std::cerr << "Could not queue buffer " << errno;
        return -2;
    }

    // Dequeue the buffer
    if(xioctl(fd, VIDIOC_DQBUF, info_buffer.get()) < 0)
    {
        std::cerr << "Could not dequeue the buffer, VIDIOC_DQBUF\n";
        return -3;
    }

    // Frames get written after dequeuing the buffer
    frame -> assignFrame(frame_buffer, info_buffer, this -> width, this -> height);

    return 0;
}
