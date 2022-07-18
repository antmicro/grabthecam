#pragma once

#include <fcntl.h>
#include <libv4l2.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "camera-capture/consts.hpp"
#include "camera-capture/frame.hpp"

/**
* Handles capturing frames from v4l cameras
* Provides C++ API for changing camera settings and capturing frames
* See how it can be used in src/example.cpp
*/
class Camera
{
public:
    /**
    * Open the Camera
    *
    * On error throws CameraException
    * @param filename Path to the camera file
    */
    Camera(std::string filename);

    /**
    * Obtain information about driver and hardware capabilities.
    *
    * On error throws CameraException
    * @param cap Structure which will be filled by the driver
    */
    void getCapabilities(ucap_ptr &cap);

    /**
    * Set the camera setting to a given value
    *
    * On error throws CameraException
    * @param property Ioctl code of the parameter to change
    * @param value Value for the parameter
    */
    void set(int property, double value);

    /**
    * Get the camera setting value
    *
    * On error throws CameraException
    * @param property Ioctl code of the parameter
    * @param value Variable, which will be filled with value
    */
    void get(int property, double &value);

    /**
    * Set the camera frame format to a given value
    *
    * On error throws CameraException
    * @param width Image width in pixels
    * @param height Image height in pixels
    * @param pixelformat The pixel format or type of compression (https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/pixfmt-reserved.html)
    */
    void setFormat(unsigned int width, unsigned int height, unsigned int pixelformat);

    /**
    * Captures a frame
    *
    * Fetch a frame (to the specific location) and optionally save it to file.
    * On error throws CameraException
    *
    * @param frame Pointer to the frame object, where all frame details will be stored
    * @param buffer_no Index of camera buffer where the frame will be fetched. Default = 0
    * @param number_of_buffers Number of buffers to allocate (if not allocated yet). If this number is not equal to the number of currently allocated buffers, the stream is restarted and new buffers are allocated.
    * @param locations Vector of pointers to a memory location, where frames should be placed. Its length should be equal to number of buffers. If not provided, the kernel chooses the (page-aligned) addresses at which to create the mapping. For more information see mmap documentation.
    */
    void capture(uframe_ptr &frame, int buffer_no=0, int number_of_buffers=1, std::vector<void*> locations=std::vector<void*>());

    /**
    * Overload provided for convenience.
    *
    * For more information see capture.
    */
    void capture(uframe_ptr &frame, int buffer_no, std::vector<void*> locations);

    /**
    * Returns the camera's file descriptor
    * @returns Camera file descriptor
    */
    int getFd() {return fd;}

    /**
    * Close the camera
    */
    ~Camera();

private:
    /**
    * Get current width and height. Sets relevants fields.
    *
    * On error throws CameraException
    */
    void updateFormat();

    /*
    * Ask the device for the buffers to capture frames and allocate memory for them
    *
    * On error throws CameraException
    * @param n Number of buffers to allocate
    * @param locations Pointers to a place in memory where frame should be placed. Its lenght should be equal to n. If not provided, the kernel chooses the (page-aligned) address at which to create the mappings. For more information see mmap documentation.
    */
    void requestBuffers(int n=1, std::vector<void*> locations=std::vector<void*>());

    /**
    * Stop streaming, free the buffers and mark camera as not ready to capture
    *
    * On error throws CameraException
    */
    void stopStreaming();

    int fd; ///< A file descriptor to the opened camera
    int width; ///< Frame width in pixels, currently set on the camera
    int height; ///< Frame width in pixels, currently set on the camera
    bool ready_to_capture; ///< If the buffers are allocated and stream is active
    svbuf_ptr info_buffer; ///< Informations about the current buffer
    int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE; ///< Type of the allocated buffer
    std::vector<fbi_ptr> buffers;  ///< Currently allocated buffers
};
