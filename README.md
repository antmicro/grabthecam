# grabthecam

Copyright (c) 2022 [Antmicro](https://www.antmicro.com)

A C++ library for controlling video4linux cameras and capturing frames.

The library provides low-level access to all camera properties with the convenience of high-level API.
Its capabilities include:

- managing the camera properties using ioctl codes
- configuring the frame format
- fetching frames (also to the specific location in memory)
- preprocessing raw frames
- saving camera configuration to file

## Building the project

The project requires:

* [OpenCV](https://opencv.org/releases/) library
* [RapidJSON](https://rapidjson.org/) library
* C++ compiler with C++20 support.

To build the project, go to its root directory and execute:

```
cmake -s . -B build
```

## Running the demo

After the successful build, you can run the demo. E.g.:

```
cd build
./grabthecam-demo --type YUYV --dims 960,720 --out frame --save ".my_configuration"
```
YUYV is the pixel type supported by the camera, and 960x720 is the frame format;

The captured frame will be saved as `frame.png`.

The configuration will be saved as `.my_configuration`. If you use `-s` it is saved as `.pyvidctrl_<driver_name>` (for compatibility with [pyvidctrl camera management TUI tool](https://github.com/antmicro/pyvidctrl)).

You can find more information about available arguments in command-line help:

```
./grabthecam-demo --help
```

## Installation

To install the library, go to the build directory and run:

```
sudo make install
```

## Usage

All functions and classes for `grabthecam` library are in the `grabthecam` C++ namespace.

The core of the library is the `CameraCapture` class. Use it for adjusting the camera settings, grabbing and reading frames.

If you want to preprocess the raw photo, you have to set the frame converter. Frame converters operate on OpenCV's matrices. Currently implemented converters support all formats convertible via [openCV's `cvtColor` and `demosaicing` functions][cv_colors] (including many types of YUV and Bayer frames).

### Create and setup an instance of `CameraCapture`

#### Create the instance of CameraCapture

The constructor will:
- open the camera "/dev/video0" device and set a file descriptor for it
- read the current frame format

You can check the value of this fields via `camera.getFd()` and `camera.getFormat()` methods.

```c++
#include <grabthecam/cameracapture.hpp>

grabthecam::CameraCapture camera("/dev/video0");
```

#### Set format of frames

- set frame resolution to 960x720
- set the color model for the raw camera frame. You can check formats supported by your camera, by running `v4l2-ctl --list-formats` in the terminal. Format identifier comes from the v4l2 library. Available formats are listed [here](https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/pixfmt-reserved.html)

```c++
camera.setFormat(960, 720, V4L2_PIX_FMT_YYUV);
```

#### Create a converter and assign it to the camera

When you set a `FrameConverter` for the `CameraCapture` object, it can return frames in format, which you can easily open (e.g. RGB).

`FrameConverter` is a base, abstract class. To assign the converter, choose `Raw2YuvConverter`, `Raw2BayerConverter` or `AnyFormat2bgrConverter`. You can also add your own converter. See the instruction for it in [Extending raw frame converters
](#Extending raw frame converters) section.

- `Raw2YuvConverter` uses the [cv::cvtColor][cv_colors] method, adjusted to YUV format. You have to pass [color space conversion code][cv_colors] for your YUV format and optionally datatype for a destination matrix. The default type is `CV_8UC3`, which means the output will have three layers (R, G, B) with 8 bits unsigned type.
- `Raw2BayerConverter` uses [cv::demosaicing][cv_colors] method. It also needs the conversion code and the type of the destination matrix.
- `AnyFormat2bgrConverter` is the most generic one. It's very similar to the `Raw2YuvConverter` but allows you to adjust `nChannels`.

You can find more information about color conversion in OpenCV in the [cvtColor function documentation](https://docs.opencv.org/3.4/d8/d01/group__imgproc__color__conversions.html#ga397ae87e1288a81d2363b61574eb8cab).

```c++
#include <grabthecam/frameconverters/raw2yuvconverter.hpp>
#include <opencv2/imgproc.hpp>

std::shared_ptr<grabthecam::FrameConverter> converter = std::make_shared<grabthecam::Raw2YuvConverter>(cv::COLOR_YUV2BGR_YUY2);
camera.setConverter(converter);
```

### Change the camera settings

The library allows to manage all properties supported by the camera. You can check them by running `v4l2-ctl --list-ctrls` or executing `camera.printControls()` method. You can get and set the controls using [codes from the V4l2 library](https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/control.html).

```c++
#include <iostream>

// Print all available controls
camera.printControls();

// Get the camera property
int value;                       // it should correspond to the property type
camera.get(V4L2_CID_BRIGHTNESS, value);
std::cout << value << std::endl; // the variable is filled with the current value of brightness

// Set the camera property
camera.set(V4L2_CID_BRIGHTNESS, 128);
```

Besides changing properties, you can run all ioctl codes (including custom ones). E.g.:

```c++
// Run different ioctl code
int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
camera.runIoctl(VIDIOC_STREAMOFF, &buffer_type);
```

### Save and load the camera settings

To save time, you can save the camera configuration to the file. Simply run:

```c++
camera.saveConfig();
```
It will save the configuration to a `.pyvidctrl_<driver_name>` JSON file. You can also provide your filename and location as an argument.

To load the saved configuration run:

```c++
camera.loadConfig();
```
You can provide your filename as well.

The configuration format is fully compatible with [pyvidctrl](https://github.com/antmicro/pyvidctrl), so you can adjust the controls using TUI if you prefer.

### Capture and save a raw frame

We can cleave off several stages of capturing the frame:
1. `grab` – fetch a frame to the buffer of the camera
1. `read` – read the frame from the camera to the computer's memory
1. preprocess – eg. convert frame from one color space to another (optional)

Hence, the process of capturing raw frame will look as follows:

```c++
#include <grabthecam/utils.hpp>

std::shared_ptr<grabthecam::MMapBuffer> raw_frame; // here the frame will be stored
camera.grab();                         // fetch the frame to camera's buffer 0
camera.read(raw_frame);                // read the content from the buffer
rawToFile("frame.raw", raw_frame);     // save it to the file
```

### Capture and save a frame

When the converter is set, you can grab, read and preprocess a frame using the `capture` method.

```c++
cv::Mat frame = camera.capture(CV_8UC2); // CV_8UC2 is the format of the raw frame matrix (this one is e.g. for YUYV format).

// Save the frame
grabthecam::saveToFile("frame.png", frame);
```

## Extending raw frame converters

The frame converters, available in the library, can preprocess raw frames. Currently, we support all formats convertible via [openCV's `cvtColor` and `demosaicing` functions][cv_colors].

To add a new converter, simply create a new class, inheriting from `FrameConverter`. It should implement the `convert` method.

If you would like to use your converter in the demo, go to the `src/example.cpp`. Add the converter to options, `pix_formats` and create a case for it in `setConverter` function. You should also add your cpp file to `CMakeLists.txt`, to allow automatic build.

## Licensing

The sources are published under the Apache 2.0 License, except for files located in the `third-party/` directory. For those files, the license is either enclosed in the file header or a separate LICENSE file.

[cv_colors]: https://docs.opencv.org/3.4/d8/d01/group__imgproc__color__conversions.html
