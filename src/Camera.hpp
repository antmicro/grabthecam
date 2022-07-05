#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <libv4l2.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <memory>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string>

class Camera{
public:
  Camera(std::string filename); /**< Open the camera*/
  void release(); /**< Close the camera */

  /**
  * Obtain information about driver and hardware capabilities.
  * @param cap structure filled by the driver
  */
  void getCapabilities(std::unique_ptr<v4l2_capability> & cap);

  /**
  * Set the camera setting to a given value
  * @param property Ioctl code of the parameter to change
  * @param value Value for the parameter
  */
  void set(int property, double value);

  /**
  * Set the camera setting to a given value
  * @param width Image width in pixels
  * @param heigh Image height in pixels
  * @param pixelformat The pixel format or type of compression (https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/pixfmt-reserved.html)
  */
  void setFormat(unsigned int width, unsigned int height, unsigned int pixelformat);
  double get(int property);
  char* capture(std::string filename);


private:
  int fd;
  char* requestBuffer();
  void saveFrameToFile(char* deviceBuff, std::unique_ptr<v4l2_buffer>& bufferinfo, std::string filename);
  void saveFrameToMemoryLocation(char* buffer, std::unique_ptr<v4l2_buffer>& bufferinfo); //TODO
};
