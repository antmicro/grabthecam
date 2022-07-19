#pragma once

#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

#include <iostream>
#include <memory>


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
    CameraException(std::string msg): msg(msg){}

    /**
     * Returns the explanatory string.
     */
    const char * what() const throw() override
    {
        return msg.c_str();
    }

private:
    std::string msg; ///< description
};
