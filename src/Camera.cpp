#include "Camera.hpp"

int xioctl(int fd, int request, void *arg)
{
    int res;
    do
    {
        res = ioctl(fd, request, arg);
    } while (-1 == res && EINTR == errno); //A signal was caught

    return res;
}


Camera::Camera(std::string filename)
{
  //Open the device

  this->fd = open(filename.c_str(), O_RDWR);
  if(fd < 0){
      std::cerr<<"Failed to open the device";
  }
}

void Camera::getCapabilities(std::unique_ptr<v4l2_capability>& cap)
{
  // Ask the device if it can capture frames
  if (xioctl(this->fd, VIDIOC_QUERYCAP, cap.get()) < 0)
  {
      if (errno == EINVAL) {
          std::cerr<< "This is not a V4L2 device\n";
      } else {
          std::cerr<< "Error in ioctl VIDIOC_QUERYCAP\n";
      }
  }
}

void Camera::set(int prop, double value)
{
}

void Camera::setFormat(unsigned int width, unsigned int height, unsigned int pixelformat)
{
  //Set Image format
  v4l2_format fmt = {0};

  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width       = width;
  fmt.fmt.pix.height      = height;
  fmt.fmt.pix.pixelformat = pixelformat;
  fmt.fmt.pix.field       = V4L2_FIELD_NONE;
  if (xioctl(this-> fd, VIDIOC_S_FMT, &fmt) < 0)
  {
    std::cerr<<"VIDIOC_S_FMT failed\n";
  }
  else
    std::cout<<"Format set"<<std::endl;
}

void Camera::release()
{
  close(this->fd);
}

char* Camera::requestBuffer()
{
  // Request Buffer from the device, which will be used for capturing frames
  struct v4l2_requestbuffers requestBuffer = {0};
  requestBuffer.count = 1;
  requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  requestBuffer.memory = V4L2_MEMORY_MMAP;

  if (xioctl(this->fd, VIDIOC_REQBUFS, &requestBuffer) < 0)
  {
    std::cerr<<"Requesting Buffer failed\n";
  }

  // ask for the you requested buffer and allocate memory for it
  struct v4l2_buffer queryBuffer = {0};
  queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  queryBuffer.memory = V4L2_MEMORY_MMAP;
  queryBuffer.index = 0;
  if(xioctl(this->fd, VIDIOC_QUERYBUF, &queryBuffer) < 0)
  {
      std::cerr<<"Device did not return the queryBuffer information\n";
  }

  // use a pointer to point to the newly created queryBuffer
  // map the memory address of the device to an address in memory
  char* buffer = (char*)mmap(NULL, queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                      this->fd, queryBuffer.m.offset);
  memset(buffer, 0, queryBuffer.length);


  //TODO: make shared_ptr
  // char* a = new char('a');
  //
  // std::shared_ptr<char> bufferPtr;
  //
  // bufferPtr.reset(a);
  // std::cout<<bufferPtr<<std::endl;
  //
  // bufferPtr.reset(buffer);

  return buffer;
}

void Camera::saveFrame(char* buffer, std::unique_ptr<v4l2_buffer>& bufferinfo, std::string filename)
{
  // Write the data out to file
  std::ofstream outFile;
  outFile.open(filename, std::ios::binary| std::ios::app);

  int bufPos = 0;   // the position in the buffer
  int outFileMemBlockSize = 0;  //the amount to copy from the buffer
  int remainingBufferSize = bufferinfo->bytesused; // the remaining buffer size

  std::unique_ptr<char> outFileMemBlock;

  while(remainingBufferSize > 0) {
    bufPos += outFileMemBlockSize;

    outFileMemBlockSize = 1024;    // output block size (To set up)

    outFileMemBlock.reset(new char[sizeof(char) * outFileMemBlockSize]);

    memcpy(outFileMemBlock.get(), buffer+bufPos, outFileMemBlockSize);
    outFile.write(outFileMemBlock.get(), outFileMemBlockSize);

    // calculate the amount of memory left to read (in case we are about to read too much)
    if(outFileMemBlockSize > remainingBufferSize)
        outFileMemBlockSize = remainingBufferSize;

    remainingBufferSize -= outFileMemBlockSize;
  }
  outFile.close();
  std::cout<<"Saved as "<<filename<<std::endl;
}


char* Camera::capture(std::string filename="")
{
  char* buffer = requestBuffer(); // buffer in the device memory

  std::unique_ptr<v4l2_buffer> bufferinfo = std::make_unique<v4l2_buffer>();

  memset(bufferinfo.get(), 0, sizeof(bufferinfo));
  bufferinfo->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  bufferinfo->memory = V4L2_MEMORY_MMAP;
  bufferinfo->index = 0;

  // Activate streaming
  int type = bufferinfo->type;
  if(xioctl(this->fd, VIDIOC_STREAMON, &type) < 0){
      std::cerr<<"Could not start streaming\n";
  }

  // Queue the buffer
  if(xioctl(fd, VIDIOC_QBUF, bufferinfo.get()) < 0){
      std::cerr<<"Could not queue buffer\n";
  }

  // Dequeue the buffer
  if(xioctl(fd, VIDIOC_DQBUF, bufferinfo.get()) < 0){
      std::cerr<<"Could not dequeue the buffer, VIDIOC_DQBUF\n";
  }
  // Frames get written after dequeuing the buffer

  if (filename != "")
  saveFrame(buffer, bufferinfo, filename);
  munmap(buffer, bufferinfo->length);

  return buffer;
}
