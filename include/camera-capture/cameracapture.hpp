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

#include <cstddef>
#include <concepts>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "camera-capture/frameconverter.hpp"
#include "camera-capture/mmapbuffer.hpp"
#include "camera-capture/utils.hpp"
#include <opencv2/core/mat.hpp>

template<typename T>
concept Numeric = std::integral<T> or std::floating_point<T>;

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
     * Run an Ioctl with a given value
     *
     * On error throws CameraException
     * @param ioctl Ioctl code to run
     * @param value Value for the parameter
     */
    template <typename T>
    int set(int ioctl, T *value);

    /*
     * Set camera setting to a given value
     *
     * On error throws CameraException
     * @param property Ioctl code of the parameter to change
     * @param value Value for the parameter
     */
    template <Numeric T>
    int set(int property, T value);

    /**
     * Get the camera setting value
     *
     * On error throws CameraException
     * @param ioctl Ioctl code to run
     * @param value Variable, which will be filled with value
     */
    template<typename T>
    int get(int ioctl, T *value) const;

    /**
     * Get the camera setting value
     *
     * On error throws CameraException
     * @param property Ioctl code of the parameter
     * @param value Variable, which will be filled with value
     * 
     * @return 
     */
    template <Numeric T>
    int get(int property, T *value) const;

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
    void grab(int buffer_no = 0, int number_of_buffers = 1, std::vector<void *> locations = std::vector<void *>());

    /**
     * Return raw frame data
     *
     * @param frame MMapBuffer where the raw frame data will be placed
     * @param buffer_no Index of camera buffer from  where the frame will be fetched. Default = 0
     */
    void read(std::shared_ptr<MMapBuffer> &frame, int buffer_no = 0) const;

    /**
     * Return raw frame data
     *
     * @param frame cv::Mat object wrapping the raw frame
     * @param dtype OpenCV's primitive datatype, in which values in matrix will be stored (see
     * https://docs.opencv.org/4.x/d1/d1b/group__core__hal__interface.html#ga78c5506f62d99edd7e83aba259250394)
     * @param buffer_no Index of camera buffer from  where the frame will be fetched. Default = 0
     */
    void read(std::shared_ptr<cv::Mat> &frame, int dtype, int buffer_no = 0) const;

    /**
     * Return raw frame data
     *
     * @param frame cv::Mat object wrapping the raw frame
     * @param dtype OpenCV's primitive datatype, in which values in matrix will be stored (see
     * https://docs.opencv.org/4.x/d1/d1b/group__core__hal__interface.html#ga78c5506f62d99edd7e83aba259250394)
     * @param buffer_no Index of camera buffer from  where the frame will be fetched. Default = 0
     */
    void read(cv::Mat *frame, int dtype, int buffer_no = 0) const;

    /**
     * Grab, export to cv::Mat (and preprocess) frame
     *
     * Grab the frame to designated buffer and read it to cv::Mat. If converter is set, convert the frame using this
     * converter. If not, ommit preprocessing.
     *
     * @param raw_frame_dtype OpenCV's primitive datatype, in which values in matrix will be stored (see
     * https://docs.opencv.org/4.x/d1/d1b/group__core__hal__interface.html#ga78c5506f62d99edd7e83aba259250394)
     * @param buffer_no Index of camera buffer from  where the frame will be fetched. Default = 0
     * @param number_of_buffers Number of buffers to allocate (if not allocated yet). If this number is not equal to the
     * number of currently allocated buffers, the stream is restarted and new buffers are allocated.
     * @param locations Vector of pointers to a memory location, where frames should be placed. Its length should be
     * equal to number of buffers. If not provided, the kernel chooses the (page-aligned) addresses at which to create
     * the mapping. For more information see mmap documentation.
     *
     * @return Captured (and preprocessed) frame
     */
    cv::Mat capture(int raw_frame_dtype, int buffer_no = 0, int number_of_buffers = 1,
                    std::vector<void *> locations = std::vector<void *>());

    //------------------------------------------------------------------------------------------------
    /**
     * Sets converter for raw frames
     * @param converter Converter object
     */
    void setConverter(std::shared_ptr<FrameConverter> converter) { this->converter = converter; }

    /**
     * Returns the camera's file descriptor
     * @returns Camera file descriptor
     */
    int getFd() { return fd; }

    /**
     * Returns current width and height
     *
     * @return width and height currently set in the camera
     */
    std::pair<int, int> getFormat() const;

    /**
     * Close the camera
     */
    ~CameraCapture();

private:

    /**
     * Check if the camera supports the property
     *
     * @param property Property to check
     * @return Result of the VIDIOC_QUERYCTL
     */
    int queryProperty(int property) const;

    /**
     * Get current width and height. Set relevants fields.
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

    int fd;                                           ///< A file descriptor to the opened camera
    int width;                                        ///< Frame width in pixels, currently set on the camera
    int height;                                       ///< Frame width in pixels, currently set on the camera
    bool ready_to_capture;                            ///< If the buffers are allocated and stream is active
    std::shared_ptr<v4l2_buffer> info_buffer;         ///< Informations about the current buffer
    int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;    ///< Type of the allocated buffer
    std::vector<std::shared_ptr<MMapBuffer>> buffers; ///< Currently allocated buffers
    std::shared_ptr<FrameConverter> converter;        ///< Converter for raw frames
};
