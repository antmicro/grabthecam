#include "camera-capture/framebufferinfo.hpp"
#include "camera-capture/utils.hpp"

FrameBufferInfo::FrameBufferInfo(void* location, int size, int fd, int offset): size(size){
    start = mmap(location, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
    memset(start, 0, size);

    if (start == (void *) -1)
    {
        throw CameraException("Mmap failed");
    }
}

FrameBufferInfo::~FrameBufferInfo()
{
    munmap(start, size);
}
