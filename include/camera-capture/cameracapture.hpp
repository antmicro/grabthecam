#pragma once

#include <fcntl.h>
#include <libv4l2.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "camera-capture/mmapbuffer.hpp"
#include <opencv2/core/mat.hpp>
#include "camera-capture/utils.hpp"
#include "camera-capture/frameconverter.hpp"

/**
 * Handles capturing frames from v4l cameras
 * Provides C++ API for changing camera settings and capturing frames
 * See how it can be used in src/example.cpp
 */
class CameraCapture
{
public:
    /**
     * Open the Camera
     *
     * On error throws CameraException
     * @param filename Path to the camera file
     */
    CameraCapture(std::string filename);

    /**
     * Obtain information about driver and hardware capabilities.
     *
     * On error throws CameraException
     * @param cap Structure which will be filled by the driver
     */
    void getCapabilities(std::unique_ptr<v4l2_capability> &cap);

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
     * @param pixelformat The pixel format or type of compression
     * (https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/pixfmt-reserved.html)
     */
    void setFormat(unsigned int width, unsigned int height, unsigned int pixelformat);


    //--------------------------------------------------------------------------------------------------

    /**
     * Fetch a frame to the buffer
     *
     * @param buffer_no Index of camera buffer where the frame will be fetched. Default = 0
     * @param number_of_buffers Number of buffers to allocate (if not allocated yet). If this number is not equal to the
     * number of currently allocated buffers, the stream is restarted and new buffers are allocated.
     * @param locations Vector of pointers to a memory location, where frames should be placed. Its length should be
     * equal to number of buffers. If not provided, the kernel chooses the (page-aligned) addresses at which to create
     * the mapping. For more information see mmap documentation.
     */
    void grab(int buffer_no=0, int number_of_buffers=1, std::vector<void *> locations = std::vector<void *>());

    /**
     * Return raw frame data
     *
     * @param frame MMapBuffer where the raw frame data will be placed
     * @param buffer_no Index of camera buffer from  where the frame will be fetched. Default = 0
     */
    void read(std::shared_ptr<MMapBuffer> &frame, int buffer_no = 0);

    /**
     * Return raw frame data
     *
     * @param frame cv::Mat where the raw frame data will be placed
     * @param datatype OpenCV's primitive datatype, in which values in matrix will be stored (see
     * https://docs.opencv.org/4.x/d1/d1b/group__core__hal__interface.html#ga78c5506f62d99edd7e83aba259250394)
     * @param buffer_no Index of camera buffer from  where the frame will be fetched. Default = 0
     */
    void read(std::shared_ptr<cv::Mat> &frame, int dtype, int buffer_no = 0);

    /**
     * Grab, export to cv::Mat and preprocess frame
     */
    void capture();

    //------------------------------------------------------------------------------------------------
    void setConverter(std::shared_ptr<FrameConverter> converter){ this->converter = converter; }

    /**
     * Returns the camera's file descriptor
     * @returns Camera file descriptor
     */
    int getFd() { return fd; }

    /**
     * Returns current width and height
     */
    int* getFormat();

    /**
     * Close the camera
     */
    ~CameraCapture();

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
     * @param locations Pointers to a place in memory where frame should be placed. Its lenght should be equal to n. If
     * not provided, the kernel chooses the (page-aligned) address at which to create the mappings. For more information
     * see mmap documentation.
     */
    void requestBuffers(int n = 1, std::vector<void *> locations = std::vector<void *>());

    /**
     * Stop streaming, free the buffers and mark camera as not ready to capture
     *
     * On error throws CameraException
     */
    void stopStreaming();

    int fd;                                        ///< A file descriptor to the opened camera
    int width;                                     ///< Frame width in pixels, currently set on the camera
    int height;                                    ///< Frame width in pixels, currently set on the camera
    bool ready_to_capture;                         ///< If the buffers are allocated and stream is active
    std::shared_ptr<v4l2_buffer> info_buffer;      ///< Informations about the current buffer
    int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE; ///< Type of the allocated buffer
    std::vector<std::shared_ptr<MMapBuffer>> buffers;                  ///< Currently allocated buffers
    std::shared_ptr<FrameConverter> converter;
};
