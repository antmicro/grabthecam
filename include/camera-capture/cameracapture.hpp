#pragma once

#include <linux/videodev2.h>

#include "camera-capture/frameconverter.hpp"
#include <concepts>
#include <opencv2/core/mat.hpp> // cv::Mat

template <typename T> concept Numeric = std::integral<T> or std::floating_point<T>;

/**
 * Handles capturing frames from v4l cameras
 * Provides C++ API for changing camera settings and capturing frames.
 *
 * See how it can be used in src/example.cpp
 */
class CameraCapture
{
public:
    /**
     * Open the Camera
     *
     * @throws CameraException
     * @param filename Path to the camera file
     */
    CameraCapture(std::string filename);

    /**
     * Set camera setting to a given value
     *
     * @throws CameraException
     *
     * @tparam Type of the parameter value. Should be numeric, i.e. int, float, double, bool...
     * @param property Ioctl code of the parameter to change
     * @param value Value for the parameter
     * @param warning Print warning to stderr when the value was clamped
     */
    template <Numeric T> void set(int property, T value, bool warning = true);

    /**
     * Run ioctl code
     *
     * @throws CameraException
     * @param ioctl Ioctl code to run
     * @param value Structure, which will be used in this execution
     */
    void runIoctl(int ioctl, void *value) const;

    /**
     * Get the camera setting value
     *
     * @throws CameraException
     *
     * @tparam Type of the parameter value. Should be numeric, i.e. int, float, double, bool...
     * @param property Ioctl code of the parameter
     * @param value Numeric (int, float, bool...) variable, which will be filled with value
     * @param current Whether to get currently set value. If it's set to false, the default parameter's value is returned
     */
    template <Numeric T> void get(int property, T &value, bool current = true) const;

    /**
     * Set the camera frame format to a given value
     *
     * @throws CameraException
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
    void read(cv::Mat &frame, int dtype, int buffer_no = 0) const;

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
     * @throws CameraException
     * @param property Property to check
     * @param query The structure, where the results should be stored. It should be empty.
     */
    void queryProperty(int property, v4l2_queryctrl &query) const;

    /*
     * Set camera setting to a given value
     *
     * @throws CameraException
     *
     * @param property Ioctl code of the parameter to change
     * @param ctrl Control stucture array with the value for the parameter filled
     * @param warning Whether to print warning to stderr when the value was clamped
     */
    void setCtrl(int property, v4l2_ext_control *ctrl, bool warning = true);

    /**
     * Get v4l2 structure with camera property
     *
     * @throws CameraException
     * @param property Ioctl code of the parameter
     * @param current Whether to get currently set value. If it's set to false, the default parameter's value is returned
     * @param ctrls Structure, which will be filled with the parameter's value
     */
    void getCtrls(int property, bool current, v4l2_ext_controls &ctrls) const;

    /**
     * Get current width and height. Set relevants fields.
     *
     * @throws CameraException
     */
    void updateFormat();

    /*
     * Ask the device for the buffers to capture frames and allocate memory for them
     *
     * @throws CameraException
     * @param n Number of buffers to allocate
     * @param locations Pointers to a place in memory where frame should be placed. Its lenght should be equal to n. If
     * not provided, the kernel chooses the (page-aligned) address at which to create the mappings. For more information
     * see mmap documentation.
     */
    void requestBuffers(int n = 1, std::vector<void *> locations = std::vector<void *>());

    /**
     * Stop streaming, free the buffers and mark camera as not ready to capture
     *
     * @throws CameraException
     */
    void stopStreaming();

    /**
     * Check if the buffer is available for read
     *
     * @param buffer_no Index of the camera buffer
     */
    void checkBuffer(int buffer_no) const;

    int fd;                                           ///< A file descriptor to the opened camera
    int width;                                        ///< Frame width in pixels, currently set on the camera
    int height;                                       ///< Frame width in pixels, currently set on the camera
    bool ready_to_capture;                            ///< If the buffers are allocated and stream is active
    std::shared_ptr<v4l2_buffer> info_buffer;         ///< Informations about the current buffer
    int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;    ///< Type of the allocated buffer
    std::vector<std::shared_ptr<MMapBuffer>> buffers; ///< Currently allocated buffers
    std::shared_ptr<FrameConverter> converter;        ///< Converter for raw frames
};
