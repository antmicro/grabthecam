#include "grabthecam/mmapbuffer.hpp"
#include "grabthecam/utils.hpp"

#include <cstring>    //memset
#include <sys/mman.h> // PROT_READ...

namespace grabthecam
{

MMapBuffer::MMapBuffer(void *location, int size, int fd, int offset) : size(size)
{
    start = mmap(location, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
    memset(start, 0, size);

    if (start == (void *)-1)
    {
        throw CameraException("Mmap failed");
    }
}

MMapBuffer::~MMapBuffer() { munmap(start, size); }

};
