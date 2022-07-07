#pragma once
#include <memory>
#include <iostream>
#include <sys/mman.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

struct D
{ // deleter for the buffer ptr
    int size;
    D(int _size)
    {
        size = _size;
    }
    D(){;}
    void operator() (char* p)
    {
        // std::cout << "deleter " << size <<std::endl;
        munmap(p, size);
    }
};

using uchar_ptr = std::unique_ptr<char, D>;
using ubuf_ptr = std::unique_ptr<v4l2_buffer>;
using ucap_ptr = std::unique_ptr<v4l2_capability>;
