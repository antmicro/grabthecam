#include "camera-capture/framebufferinfo.hpp"

FrameBufferInfo::FrameBufferInfo(void* location, int size, int fd, int offset): size(size){
    start = mmap(location, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
    memset(start, 0, size);

    if (start == (void *) -1)
    {
        std::cerr<<"Mmap failed\n";
    }
}

FrameBufferInfo::~FrameBufferInfo()
{
    std::cout << "deleter " << size << std::endl;
    munmap(start, size);
}
