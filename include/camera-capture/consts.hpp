#pragma once
#include <memory>
#include <cstring> //memset
#include <iostream>
#include <sys/mman.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

/**
 * Class for managing memory mapping and keeping information about buffer.
 */
class FrameBufferInfo{
public:
    /**
     * Constructor. Maps the memory.
     * 
     * @param location Pointer to a memory location, where frame should be placed. If not provided, the kernel chooses the (page-aligned) address at which to create the mapping. For more information see mmap documentation.
     * @param size Size of the buffer to allocate
     * @param fd Camera file descriptor
     * @param offset Offset in fd. For more information see mmap documentation
     */     
    FrameBufferInfo(void* location, int size, int fd, int offset): size(size){   
        start = mmap(location, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
        memset(start, 0, size);

        if (start == (void *) -1)
        {
            std::cerr<<"Mmap failed\n";
            //return -3; //TODO: exception?
        }
    };

    /**
     * Destructor. Unmaps the memory
     */
    ~FrameBufferInfo()
    {
        std::cout << "deleter " << size << std::endl;
        munmap(start, size);
    }
    
    unsigned int bytesused; ///< bytes used by a captured frame
    void* start; ///< pointer to the memry location, where the buffer starts
    int size; ///< size of the buffer
};

using fbi_ptr = std::shared_ptr<FrameBufferInfo>;
using ucap_ptr = std::unique_ptr<v4l2_capability>;
using schar_ptr = std::shared_ptr<char>;
using svbuf_ptr = std::shared_ptr<v4l2_buffer>;
