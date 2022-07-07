#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <libv4l2.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <memory>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string>
#include <vector>

struct D
{ // deleter for the buffer ptr
    int size;
    D(int _size)
    {
        size = _size;
    }
    D(){;}
    void operator() (char* p)
    {
        std::cout << "deleter " << size <<std::endl;
        munmap(p, size);
    }
};

using uchar_ptr = std::unique_ptr<char, D>;
using ubuf_ptr = std::unique_ptr<v4l2_buffer>;
using ucap_ptr = std::unique_ptr<v4l2_capability>;


class Camera
{
public:
    /**
    * Open the Camera
    * @param filename Path to the camera file
    */
    Camera(std::string filename);

    /**
    * Close the camera
    */
    void release();

    /**
    * Obtain information about driver and hardware capabilities.
    * @param cap Structure which will be filled by the driver
    * @return Returns 0, or -1 if error occured (in which case, errno is set appropriately – like in VIDIOC_QUERYCAP).
    */
    int getCapabilities(ucap_ptr &cap);

    /**
    * Set the camera setting to a given value
    * @param property Ioctl code of the parameter to change
    * @param value Value for the parameter
    * @return Returns 0, or -1 if error occured (in which case, errno is set appropriately – like in VIDIOC_S_CTRL).
    */
    int set(int property, double value);

    /**
    * Get the camera setting value
    * @param property Ioctl code of the parameter
    * @param value Variable, which will be filled with value
    * @return Returns 0, or -1 if error occured (in which case, errno is set appropriately – like in VIDIOC_G_CTRL).
    */
    int get(int property, double &value);

    /**
    * Set the camera frame format to a given value
    * @param width Image width in pixels
    * @param heigh Image height in pixels
    * @param pixelformat The pixel format or type of compression (https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/pixfmt-reserved.html)
    */
    int setFormat(unsigned int width, unsigned int height, unsigned int pixelformat);

    /**
    * Captures a frame
    *
    * Fetches a frame (to the specific location) and optionally saves it to file.
    *
    * @param buffer Pointer to the variable where the frame will be stored
    * @param bufferLength Pointer to the variable where the size of the frame will be stored
    * @param location [Optional] Pointer to a place in memory where frame should be placed
    * @param filename [Optional] Filename, where the frame will be saved
    */
    int capture(uchar_ptr &buffer, int *bufferLength, void *location, std::string filename);

    /**
    *Returns the camera's file descriptor
    */
    int getFd();

private:
    int fd;

    int requestBuffer(uchar_ptr &buffer, void *location);
    int saveFrameToFile(const uchar_ptr &buffer, ubuf_ptr &bufferinfo, std::string filename);
};
