#pragma once

#include <linux/videodev2.h>

#include "grabthecam/frameconverter.hpp"
#include "grabthecam/utils.hpp"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <concepts>
#include <functional>
#include <opencv2/core/mat.hpp> // cv::Mat
#include <optional>
#include <type_traits>

template <typename T>
concept numeric = std::integral<T> or std::floating_point<T>;

namespace grabthecam
{
/**
 * Structure holding information about an external trigger
 */
struct TriggerInfo
{
    uint32_t mode_reg;       /// the trigger mode ioctl register address
    uint32_t source_reg;     /// the trigger source ioctl register address
    int32_t source_value;    /// source for the trigger set to the source_reg register
    uint32_t activation_reg; /// the activation mode ioctl register address
    int32_t activation_mode; /// activation mode value set to the ioctl activation_reg register
};

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
     * @brief Helper object that denotes status of property queries
     *
     */
    enum class CameraPropertyStatus
    {
        ENABLED,
        DISABLED,
        UNSUPPORTED,
    };

    /**
     * @brief Helper wrapper over v4l2_queryctrl
     *
     */
    struct CameraProperty
    {
        int32_t id;
        std::string name;
        uint32_t type;
        int64_t defaultValue;
    };

    /**
     * @brief Helper wrapper over v4l2_querymenu
     *
     */
    struct CameraPropertyMenuEntry
    {
        uint32_t index;
        std::string name;
    };

    /**
     * @brief Helper structure that can hold results of multiple queries against the property
     *
     */
    struct CameraPropertyDetails
    {
        CameraProperty property;
        std::vector<CameraPropertyMenuEntry> menuEntries;
    };

    /**
     * Returns the trigger information
     * @returns std::optional for the trigger information structure
     */
    std::optional<TriggerInfo> getTriggerInfo() { return trigger_info; }

    /**
     * Sets the trigger information
     *
     * @param new_info structure containing new trigger activation data
     */
    void setTriggerInfo(TriggerInfo new_info) { this->trigger_info = new_info; }
    /**
     * Dumps trigger information via the provided writer
     * @param trigger_info information about the trigger
     * @param writer rapidjson PrettyWritter object for the config file
     */
    void saveTriggerInfo(TriggerInfo trigger_info, rapidjson::PrettyWriter<rapidjson::StringBuffer> &writer);
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
    template <numeric T> void set(int property, T value, bool warning = true);

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
     * @param current Whether to get currently set value. If it's set to false, the default parameter's value is
     * returned
     */
    template <numeric T> void get(int property, T &value, bool current = true) const;

    /**
     * Set the camera frame format to a given value
     *
     * If the dimentons are 0 x 0, only pixel format is changed
     * @throws CameraException
     * @param width Image width in pixels
     * @param height Image height in pixels
     * @param pixelformat The pixel format or type of compression
     * (https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/pixfmt-reserved.html)
     * @param keep_converter If set to false, try to determine converter based on pixel format (see: autoSetConverter)
     */
    void setFormat(unsigned int width, unsigned int height, unsigned int pixelformat = 0, bool keep_converter = false);

    /**
     * Save configuration to file
     *
     * Save camera parameters to file, so you can load them later.
     *
     * @param filename Where to save the configuration (by default it's `.pyvidctrl-<driver_name>`)
     *
     * @return Filename where the configuration was saved
     */
    std::string saveConfig(std::string filename = "");

    /**
     * Load configuration from file
     *
     * @param filename Where the configuration is located (by default searches in the current directory for
     * `.pyvidctrl-<driver_name>` )
     *
     * @return Filename from where the configuration was loaded
     */
    std::string loadConfig(std::string filename = "");

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
     * WARNING: You shouldn't provide the type other than provided in converter, when the object has one. (If in doubt,
     * leave it with the default value -1)
     * @param buffer_no Index of camera buffer from  where the frame will be fetched. Default = 0
     * @param number_of_buffers Number of buffers to allocate (if not allocated yet). If this number is not equal to the
     * number of currently allocated buffers, the stream is restarted and new buffers are allocated.
     * @param locations Vector of pointers to a memory location, where frames should be placed. Its length should be
     * equal to number of buffers. If not provided, the kernel chooses the (page-aligned) addresses at which to create
     * the mapping. For more information see mmap documentation.
     *
     * @return Captured (and preprocessed) frame
     */
    cv::Mat capture(int raw_frame_dtype = -1, int buffer_no = 0, int number_of_buffers = 1,
                    std::vector<void *> locations = std::vector<void *>());

    //------------------------------------------------------------------------------------------------
    /**
     * Sets converter for raw frames
     * @param converter Converter object
     */
    void setConverter(std::shared_ptr<FrameConverter> converter) { this->converter = converter; }

    /**
     * Whether the object has a converter set
     *
     * @returns true if the converter is set, false otherwise
     */
    bool hasConverter() { return (bool)converter; }

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
     * Show all camera parameters
     */
    void printControls() const;

    /**
     * @brief Query information about a property
     *
     * @param propertyID Index of queried property
     * @param property A reference to a CameraProperty object to be filled with fetched data
     * @return CameraPropertyStatus
     * @throws CameraException
     */
    CameraPropertyStatus queryProperty(int32_t propertyID, CameraProperty &property) const;

    /**
     * @return Vector of CameraProperty
     * @throws CameraException
     */
    std::vector<CameraProperty> queryProperties() const;

    /**
     * @brief Query all menu entries for given property
     *
     * @param propertyID Index of queried property
     * @return std::vector<CameraPropertyMenuEntry>
     */
    std::vector<CameraPropertyMenuEntry> queryPropertyMenuEntries(int32_t propertyID) const;

    /**
     * @brief
     *
     * @param propertyID Index of queried property
     * @return CameraPropertyDetails
     * @throws CameraException
     */
    CameraPropertyDetails queryPropertyDetails(int32_t propertyID) const;

    /**
     * Sets the trigger mode for the video device
     *
     * @throws CameraException
     */
    void defaultEnableTrigger() const;
    std::function<void()> enableTrigger = std::bind(&grabthecam::CameraCapture::defaultEnableTrigger, this);

    /**
     * Sets the trigger enabling method to a provided implementation
     *
     * @param func a void() function that will enable the trigger in the video device
     */
    void setEnableTriggerFunction(std::function<void()> func) { this->enableTrigger = func; }

    /**
     * Raises an exception on a not implemented trigger
     *
     * @throws CameraException
     */
    void defaultTriggerFrame() { throw CameraException("Triggering method not provided."); }
    std::function<void()> triggerFrame = std::bind(&grabthecam::CameraCapture::defaultTriggerFrame, this);

    /**
     * Sets the trigger enabling method to a provided implementation
     *
     * @param func a void() function that will trigger the trigger in the video device
     */
    void setTriggerFrameFunction(std::function<void()> func) { this->triggerFrame = func; }

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
     * @param current Whether to get currently set value. If it's set to false, the default parameter's value is
     * returned
     * @param ctrls Structure, which will be filled with the parameter's value
     */
    void getCtrls(int property, bool current, v4l2_ext_controls &ctrls) const;

    /**
     * Get current width and height. Set relevants fields.
     *
     * @param keep_converter If set to false, try to determine converter based on pixel format (see: autoSetConverter)
     *
     * @throws CameraException
     */
    void updateFormat(bool keep_converter = false);

    /**
     * Try to determine (and set) converter based on pixel format
     *
     * @throws CameraException
     */
    void autoSetConverter();

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

    /**
     * Get control with a given id and add it as an object to a json string
     *
     * @param queryctrl A structure to fill with information about the property. The id field should be filled
     * @param json Stringstream to which the object should be appended.i
     *
     * @return 0 if the property is valid or disabled, 1 otherwise.
     */
    int saveControlValue(v4l2_queryctrl &queryctrl, rapidjson::PrettyWriter<rapidjson::StringBuffer> &writer);

    /**
     * Return the default filename for configuration
     *
     * @return ".pyvidctrl-<driver_name>"
     */
    std::string getConfigFilename();

    /**
     * Print items for menu controls
     */
    void enumerateMenu(v4l2_queryctrl &queryctrl) const;

    /**
     * Print control's name and current and default value.
     *
     * @return 0 if the property is valid or disabled, 1 otherwise.
     */
    int printControl(v4l2_queryctrl &queryctrl) const;

    int fd;                                           ///< A file descriptor to the opened camera
    int width;                                        ///< Frame width in pixels, currently set on the camera
    int height;                                       ///< Frame width in pixels, currently set on the camera
    int v4l2_format_code = 0;                         ///< V4L2_PIX_FMT code, currently set on the camera
    bool ready_to_capture;                            ///< If the buffers are allocated and stream is active
    std::shared_ptr<v4l2_buffer> info_buffer;         ///< Informations about the current buffer
    int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;    ///< Type of the allocated buffer
    std::vector<std::shared_ptr<MMapBuffer>> buffers; ///< Currently allocated buffers
    std::shared_ptr<FrameConverter> converter;        ///< Converter for raw frames
    std::optional<TriggerInfo> trigger_info;          ///< Information about the external trigger configuration
};

}; // namespace grabthecam
