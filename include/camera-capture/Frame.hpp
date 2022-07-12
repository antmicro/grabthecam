#pragma once
#include "consts.hpp"
#include <opencv2/core/mat.hpp>
#include <fstream>
#include <opencv2/imgcodecs.hpp> //imwrite

/**
* Handles captured frames
* Provides C++ API for converting images to openCV matrices, preprocessing them and saving to file.
* See how it can be used in src/example.cpp
*/
class Frame
{
public:
    /**
    * Frame constructor
    * @param raw_frame_dtype OpenCV primitive datatype, in which values in raw_frame will be stored
    */
    Frame(int _raw_frame_dtype);

    /**
    * Preprocess the raw frame
    */
    virtual int retreive() = 0;

    /**
    * Assign information about captured frame
    * @param info Information about frame buffer's location
    * @param width Image width in pixels
    * @param height Image height in pixels
    */
    void assignFrame(fbi_ptr &_info, int _width, int _heigh);

    /**
    * Save raw frame to file
    * @param filename Where the frame will be saved
    * @return Returns 0, or -1 if error occured.
    */
    int rawFrameToFile(std::string filename);

    /**
    * Save processed frame to file
    * @param filename Where the frame will be saved
    * @return Returns 0, or -1 if error occured.
    */

    int processedFrameToFile(std::string filename);

    /**
    * Get raw frame as openCV matrix
    * @return Returns raw frame
    */
    cv::Mat getRawFrame();

    /**
    * Get processed frame as openCV matrix
    * @return Returns raw frame
    */
    cv::Mat getProcessedFrame();

    fbi_ptr info; ///< Information about frame buffer's location
    int width; ///< Image width in pixels
    int height; ///< Image height in pixels

protected:
    cv::Mat processed_frame; ///< Frame after applying `retreive` method
    int raw_frame_dtype; ///< OpenCV primitive datatype, in which values in raw_frame will be stored. See: https://docs.opencv.org/4.x/d1/d1b/group__core__hal__interface.html#ga78c5506f62d99edd7e83aba259250394

private:
    /*
    * Convert the frame to an openCV matrix
    */
    void rawToCvMat();

    cv::Mat raw_frame; ///< Captured frame after conversion to cv::Mat
};

using uframe_ptr = std::unique_ptr<Frame>;
