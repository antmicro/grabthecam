#pragma once

#include "grabthecam/mmapbuffer.hpp"
#include <opencv2/core/mat.hpp> // cv::Mat

namespace grabthecam
{

/**
 * Exception to handle errors from CameraCapture
 */
class CameraException : public std::exception
{
public:
    /**
     * Constructor
     *
     * @param msg Exception description
     * @param error_code Linux error code (0 if not related)
     */
    CameraException(std::string msg, int error_code = 0) : error_code(error_code) { setMessage(msg); }

    /**
     * Combine description and error code
     * @param text Exception description
     */
    void setMessage(std::string text)
    {
        msg = (error_code != 0) ? (std::to_string(error_code) + " – " + strerror(error_code)) : "";
        msg = (text != "") ? (msg + "\n" + text) : msg;
    }

    /**
     * Returns the explanatory string.
     *
     * @return Message, which explains the error
     */
    const char *what() const throw() override { return msg.c_str(); }

    int error_code; ///< linux error code (0 if not related)

private:
    std::string msg; ///< description
};

/**
 * Check if directories exist and create them if necessary
 *
 * @param filename Path for filename which should be present in filesystem
 */
void createDirectories(std::string filename);

/**
 * Save raw frame to file
 *
 * Saves the frame as a sequence of bytes
 *
 * @param filename Where to save the file
 * @param frame The raw frame to save
 */
void rawToFile(std::string filename, std::shared_ptr<MMapBuffer> frame);

/**
 * Save processed frame to file
 *
 * Saves the file using opencv method
 *
 * @param filename Where to save the file
 * @param frame The frame to save
 */
void saveToFile(std::string filename, cv::Mat &frame);

};
