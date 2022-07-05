#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <fstream>
#include <string>
#include <memory>

class Camera{
  int fd;
public:
  Camera(std::string filename);
  void getCapabilities(std::unique_ptr<v4l2_capability> & cap);
  void release();
  void set(int prop, double value); //TODO
  void setFormat(unsigned int width, unsigned int height, unsigned int pixelformat);
  char* capture(std::string filename);


private:
  char* requestBuffer();
  void saveFrame(char* deviceBuff, std::unique_ptr<v4l2_buffer>& bufferinfo, std::string filename);
};
