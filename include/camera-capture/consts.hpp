#pragma once
#include <memory>
#include <cstring> //memset
#include <iostream>
#include <sys/mman.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>


class FrameBufferInfo{
public:
    FrameBufferInfo(void* location, int size, int fd, int offset): size(size){   
        start = mmap(location, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
        memset(start, 0, size);

        if (start == (void *) -1)
        {
            std::cerr<<"Mmap failed\n";
            //return -3; //TODO: exception?
        }
    };

    ~FrameBufferInfo()
    {
        std::cout << "deleter " << size << std::endl;
        munmap(start, size);
    }
    
    unsigned int bytesused;
    void* start;
    int size;
};

using fbi_ptr = std::shared_ptr<FrameBufferInfo>;

using ucap_ptr = std::unique_ptr<v4l2_capability>;
using schar_ptr = std::shared_ptr<char>;
using svbuf_ptr = std::shared_ptr<v4l2_buffer>;
