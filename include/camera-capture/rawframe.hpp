#pragma once
#include "frame.hpp"

using fbi_ptr = std::shared_ptr<FrameBufferInfo>;

class RawFrame : public Frame
{
public:
    /**
     * Default constructor
     */
    RawFrame();

    /** Constructor
     * @param datatype OpenCV's primitive datatype, in which values in matrix will be stored (see https://docs.opencv.org/4.x/d1/d1b/group__core__hal__interface.html#ga78c5506f62d99edd7e83aba259250394)
     */
    RawFrame(int datatype);

    /**
     * Assign information about captured frame
     * @param info Information about frame buffer's location
     * @param width Image width in pixels
     * @param height Image height in pixels
     */
    void assignFrame(fbi_ptr &info, int width, int height);

    /**
     * [TEMPORARY] Read raw frame from file
     * @param mat_dtype OpenCV's primitive datatype, in which values in matrix will be stored
     */
    void readFromFile(std::string filename, int width, int height, int mat_dtype=CV_8UC1);

    /**
     * Save raw frame to file
     *
     * On error throws CameraException
     * @param filename Where the frame will be saved
     */
    void saveToFile(std::string filename) override;

    /**
     * Get raw frame as openCV matrix
     * @return Returns raw frame
     */
    cv::Mat getMatrix() override;

    fbi_ptr info; ///< Information about frame buffer's location
    int width; ///< Image width in pixels
    int height; ///< Image height in pixels
    int dtype; ///< OpenCV's primitive datatype, in which values in matrix will be stored (see constructor)

private:
    /*
     * Convert the frame to an openCV matrix
     */
    void rawToCvMat();
};
