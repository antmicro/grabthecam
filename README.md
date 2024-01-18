# grabthecam

Copyright (c) 2022-2023 [Antmicro](https://www.antmicro.com)

A C++ library for controlling video4linux cameras and capturing frames.

The library provides low-level access to all camera properties with the convenience of high-level API.
Its capabilities include:

- camera properties management using ioctl codes
- frame format configuration
- frame fetching (also to a specific location in memory)
- raw frame preprocessing
- saving camera configuration to file

## Building the project

Project requirements:

* [OpenCV](https://opencv.org/releases/) library
* [RapidJSON](https://rapidjson.org/) library
* [Video4Linux](https://github.com/philips/libv4l) library
* C++ compiler with C++20 support.

To build the project, go to its root directory and execute:

```
cmake -S . -B build
```

In order to add the option to build a `grabthecam` - `farshow` integration demo, execute the following command instead:
```
cmake -S . -B build -DADD_GRABTHECAM_FARSHOW_DEMO=ON
```

Next, go to `build` and execute either
```
make
```
for the standalone demonstration, or
```
make grabthecam-farshow-streamer
```
for the integration demo.

## Running the demo

After a successful build, you can run the demo, e.g.:

```
cd build
./grabthecam-demo --type YUYV --dims 960,720 --out frame --save=.my_configuration
```
YUYV is the pixel type supported by the camera, and 960x720 is the frame format;

The captured frame will be saved as `frame.png`.

The configuration will be saved as `.my_configuration`. If you use `-s` it is saved as `.pyvidctrl_<driver_name>` (for compatibility with [pyvidctrl camera management TUI tool](https://github.com/antmicro/pyvidctrl)).
Please note that `--save` and `--load` can only have values assigned through the `--param=value` syntax.

Farshow-Grabthecam integration demo follows simmilar syntax, with an addition of `-a <address>` and `-p <port>` for configuring the frame destination for the sender
```
cd build
./grabthecam-farshow-streamer --type YUYV --dims 960,720 -a 0.0.0.0 -p 18881
```

To start the receiver, please follow the README instructions on installation from the [farshow](https://github.com/antmicro/farshow) repository and run
```(bash)
farshow -i 0.0.0.0 -p 18881
```

You can find more information about available arguments in command-line help:

```
./grabthecam-demo --help
./grabthecam-farshow-streamer --help
```

## Installation

To install the library, go to the build directory and run:

```
sudo make install
```

## Usage

All functions and classes for `grabthecam` library are in the `grabthecam` C++ namespace.

The core of the library is the `CameraCapture` class. Use it to adjust camera settings, grab and read frames.

If you want to preprocess a raw photo, you need to set a frame converter. Frame converters operate on OpenCV's matrices. Currently implemented converters support all formats convertible via [openCV's `cvtColor` and `demosaicing` functions][cv_colors] (including many types of YUV and Bayer frames).

### Create and setup an instance of `CameraCapture`

#### Create an instance of CameraCapture

The constructor will:
- open the camera "/dev/video0" device and set a file descriptor for it
- read the current frame format

You can check the value of this fields via `camera.getFd()` and `camera.getFormat()` methods.

```c++
#include <grabthecam/cameracapture.hpp>

grabthecam::CameraCapture camera("/dev/video0");
```

#### Set frame format

- set frame resolution to 960x720
- set the color model to a raw camera frame. You can check formats supported by your camera, by running `v4l2-ctl --list-formats` in the terminal. The format identifier comes from the v4l2 library. Available formats are listed [here](https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/pixfmt-reserved.html)

```c++
camera.setFormat(960, 720, V4L2_PIX_FMT_YYUV);
```

#### Create a converter and assign it to the camera

When you set up a `FrameConverter` for the `CameraCapture` object, it can return frames in a format which you can easily open (e.g. RGB).

`FrameConverter` is a base, abstract class. To assign the converter, choose `Raw2YuvConverter`, `Raw2BayerConverter` or `AnyFormat2bgrConverter`. You can also add your own converter - see the instructions in the [Extending raw frame converters
](#Extending raw frame converters) section.

- `Raw2YuvConverter` uses the [cv::cvtColor][cv_colors] method, adjusted to the YUV format. You need to pass [color space conversion code][cv_colors] for your YUV format and optionally datatype for a destination matrix. The default type is `CV_8UC3`, which means the output will have three layers (R, G, B) with 8 bits unsigned type.
- `Raw2BayerConverter` uses the [cv::demosaicing][cv_colors] method. It also needs the conversion code and the type of the destination matrix.
- `AnyFormat2bgrConverter` is the most generic one. It's very similar to the `Raw2YuvConverter` but allows you to adjust `nChannels`.

You can find more information about color conversion in OpenCV in the [cvtColor function documentation](https://docs.opencv.org/3.4/d8/d01/group__imgproc__color__conversions.html#ga397ae87e1288a81d2363b61574eb8cab).

```c++
#include <grabthecam/frameconverters/raw2yuvconverter.hpp>
#include <opencv2/imgproc.hpp>

std::shared_ptr<grabthecam::FrameConverter> converter = std::make_shared<grabthecam::Raw2YuvConverter>(cv::COLOR_YUV2BGR_YUY2);
camera.setConverter(converter);
```

### Change camera settings

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

Besides changing properties, you can run all ioctl codes (including custom ones), e.g.:

```c++
// Run different ioctl code
int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
camera.runIoctl(VIDIOC_STREAMOFF, &buffer_type);
```

### Save and load camera settings

To save time, you can save the camera configuration to a file. Simply run:

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

We can cleave off several stages of capturing a frame:
1. `grab` – fetch a frame to the buffer of the camera
1. `read` – read the frame from the camera to the computer's memory
1. preprocess – e.g. convert frame from one color space to another (optional)

This way, the raw frame capture process will look as follows:

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

The frame converters available in the library can preprocess raw frames. Currently, we support all formats convertible via [openCV's `cvtColor` and `demosaicing` functions][cv_colors].

To add a new converter, simply create a new class, inheriting from `FrameConverter`. It will implement the `convert` method.

If you would like to use your converter in the demo, go to `src/example.cpp`. Add the converter to options, `pix_formats` and create a case for it in the `setConverter` function. You should also add your cpp file to `CMakeLists.txt` to allow automatic build.


## FrameConverter compatibility lookup

The FrameConverters are specific to raw input formats.
Specific formats can have multiple working setups, but one was tested and verified for most common types.
Those include (please note that specifying the `cv::Mat` format to `grabthecam::read()` is the best practice to ensure compatibility):

- RGB24: no converter necessary
- BGR24, MJPEG: AnyFormat2BGRConverter COLOR_BGR2RGB CV_8UC3
- RGBA32: AnyFormat2BGRConverter COLOR_RGBA2RGB CV_8UC4, CV_8UC4
- ABGR32: AnyFormat2BGRConverter COLOR_BGRA2BGR CV_8UC4
- BGRA32, ARGB32: Channel swapping issues with OpenCV internal conversion codes, no error with RGBA32 settings
- RGB332, RGB565, ARGB444, ABGR444, RGBA444, BGRA444, ARGB555, ABGR555, RGBA555, BGRA555: PackedFormats2RGBconverter PACKED_<name_of_format>
- GRAY: no converter necessary
- YUY2: Yuv2BGRConverter COLOR_YUV2RGB 
- UYVY: Yuv2BGRConverter COLOR_YUV2BGR_UYVY 
- YVYU: Yuv2BGRConverter COLOR_YUV2BGR_YVYU 
- GRAY10, GRAY12: unsupported in `v4l2`
- RGGB, BGGR, GBRG, GRBG: Bayer2BGRConverter, COLOR_Bayer<name_of_format>2BGR_EA
- RG10, RG12, RG16: Bayer2BGRConverter, COLOR_BayerRGGB2BGR_EA, CV_16UC1

For packed YCbCr formats, a rescaling factor is provided to the frame writing method.
Thus, usage of `grabthecam::read(mat_type)` is required with a specified type.

- NV12: Yuv2BGRConverter COLOR_YUV2BGR_NV12
- NV21: Yuv2BGRConverter COLOR_YUV2BGR_NV21
- YUV420: Yuv2BGRConverter COLOR_YUV2BGR_I420
- YVU420: Yuv2BGRConverter COLOR_YUV2BGR_YV12

## Licensing

The sources are published under the Apache 2.0 License, except for files located in the `third-party/` directory. For those files, the license is either enclosed in the file header or a separate LICENSE file.

[cv_colors]: https://docs.opencv.org/3.4/d8/d01/group__imgproc__color__conversions.html
