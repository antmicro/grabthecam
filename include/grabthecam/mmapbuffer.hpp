#pragma once

namespace grabthecam
{

/**
 * Class for managing memory mapping and keeping information about buffer.
 */
class MMapBuffer
{
public:
    /**
     * Constructor. Maps the memory.
     *
     * @param location Pointer to a memory location, where frame should be placed. If not provided, the kernel chooses
     * the (page-aligned) address at which to create the mapping. For more information see mmap documentation.
     * @param size Size of the buffer to allocate
     * @param fd Camera file descriptor
     * @param offset Offset in fd. For more information see mmap documentation
     */
    MMapBuffer(void *location, int size, int fd, int offset);

    /**
     * Destructor. Unmaps the memory
     */
    ~MMapBuffer();

    unsigned int bytesused; ///< bytes used by a captured frame
    void *start;            ///< pointer to the memory location, where the buffer starts
    int size;               ///< size of the buffer
};

};
