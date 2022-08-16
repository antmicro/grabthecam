#pragma once

#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

#include <iostream>
#include <memory>
#include <string.h> //strerror

/**
 * Exception to handle errors from camera Class
 */
class CameraException : public std::exception
{
public:
    /**
     * Constructor
     *
     * @param msg Exception description
     */
    CameraException(std::string msg, int error_code = 0) : error_code(error_code) { setMessage(msg); }

    void setMessage(std::string text)
    {
        msg = (error_code != 0) ? (std::to_string(error_code) + " " + strerror(error_code)) : "";
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
