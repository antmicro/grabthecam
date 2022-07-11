#pragma once
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <libv4l2.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <string>
#include <vector>

#include "consts.hpp"
#include "Frame.hpp"


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
    ~Camera();

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
    * @param frame Frame object, where all frame details will be stored
    * @param location [Optional] Pointer to a place in memory where frame should be placed
    */
    int capture(uframe_ptr &frame, int buffer_no=0, void *location=NULL);

    /**
    *Returns the camera's file descriptor
    */
    int getFd();

private:

    int fd;
    int width;
    int height;
    bool ready_to_capture;
    //schar_ptr frame_buffer;
    svbuf_ptr info_buffer;
    int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    std::vector<sbuf_ptr> buffers;

    /*
    * Get current width and height
    */
    int updateFormat();

    /*
    * Ask the device for the buffer to capture frames and allocate memory for it
    */
    int requestBuffers(int n=1, void *location=NULL);
};
