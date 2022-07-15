#pragma once
#include <memory>
#include <cstring> //memset
#include <iostream>
#include <sys/mman.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

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

using ucap_ptr = std::unique_ptr<v4l2_capability>;
using schar_ptr = std::shared_ptr<char>;
using svbuf_ptr = std::shared_ptr<v4l2_buffer>;
