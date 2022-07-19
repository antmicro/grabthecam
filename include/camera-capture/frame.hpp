#pragma once

#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp> //imwrite
#include <filesystem> // checking if the directory exists
#include <fstream>

#include "camera-capture/utils.hpp"
#include "camera-capture/framebufferinfo.hpp"


/**
 * Handles frames.
 *
 * Provides C++ API for converting images to openCV matrices and saving to file.
 * See how it can be used in src/example.cpp.
 */
class Frame
{
public:
    /**
     * Constructor
     */
    Frame(){}

    /**
     * Constructor
     * @param matrix Frame itself
     */
    Frame(cv::Mat matrix): matrix(matrix){}

    /**
     * Save frame to file
     *
     * On error throws CameraException
     * @param filename Where the frame will be saved
     */
    virtual void saveToFile(std::string filename);

    /**
     * Get frame as openCV matrix
     * @return Returns raw frame
     */
    virtual cv::Mat getMatrix();

protected:
    cv::Mat matrix; ///< frame itself
};

using uframe_ptr = std::unique_ptr<Frame>;
