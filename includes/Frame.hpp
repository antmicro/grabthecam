#pragma once
#include "consts.hpp"
#include <opencv2/core/mat.hpp>
#include <fstream>
#include <opencv2/imgcodecs.hpp> //imwrite


using cv::Mat;

class Frame
{
public:
    /**
    * Frame constructor
    * @param location Begin of memory, where the frame is stored
    * @param info Contains data exchanged by application and driver using one of the Streaming I/O methods.
    * @param width Image width in pixels
    * @param height Image height in pixels
    */
    Frame(int _raw_frame_dtype);
    sbuf_ptr buffer;
    int width;
    int height;

    /**
    * Save raw frame
    * @param filename Filename, where the frame will be saved
    */
    int rawFrameToFile(std::string filename);
    int processedFrameToFile(std::string filename);

    /*
    * Get frame as openCV matrix
    * @return Returns raw frame
    */
    Mat getRawFrame();
    Mat getProcessedFrame();

    /*
    * Preprocess the raw frame
    */
    virtual int retreive() = 0;
    void assignFrame(sbuf_ptr &_buffer, int _width, int _heigh);


protected:
    Mat processed_frame;
    int raw_frame_dtype;

private:
    Mat raw_frame;

    /*
    * Convert the frame to an openCV matrix
    */
    void rawToCvMat();

};

using uframe_ptr = std::unique_ptr<Frame>;
