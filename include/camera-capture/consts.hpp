#pragma once
#include <memory>
#include <iostream>
#include <sys/mman.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>


//TODO: change to class
struct MMAPDeleter
{ // deleter for the buffer ptr
    int size;
    MMAPDeleter(int _size)
    {
        size = _size;
    }
    MMAPDeleter(){}
    void operator() (char* p)
    {
        std::cout << "deleter " << size <<std::endl;
        munmap(p, size);
    }
};

// using uchar_ptr = std::unique_ptr<char, MMAPDeleter>;
using ucap_ptr = std::unique_ptr<v4l2_capability>;
using schar_ptr = std::shared_ptr<char>;
using svbuf_ptr = std::shared_ptr<v4l2_buffer>;


class FrameBufferInfo
{
public:
    FrameBufferInfo(size_t length_, schar_ptr start_): length(length_), start(start_){}

    unsigned int bytesused;
    size_t length;
    schar_ptr start;
};

using sbuf_ptr = std::shared_ptr<FrameBufferInfo>;
