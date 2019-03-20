
#ifndef OPENCV_SERVER_H
#define OPENCV_SERVER_H

#include "opencv2/core.hpp"

#include "core/object.h"
#include "core/os/thread.h"

class OpenCVServer : public Object {
    GDCLASS(OpenCVServer, Object)

protected:
	static void _bind_methods(); // bind the public methods to the class db

private:
    // source image
    cv::Mat source;
    int source_type;

    // destination/final image
    cv::Mat dest;
    int dest_type;
    
    // black and white image
    cv::Mat bw_img;

    int height;
    int width;

    static Thread *thread;
    static int task;
    static int a;

    Array *image_data;
    
    static void do_something(void *data);

public:
    int error_flag; // this flag is there to give more details about the failure of a process [0 = working fine]
    // this won't have any use right now hopefully I will implement it some day.. :)

    // operations will be a function that can be called as per the enum that we have sorta like notifications I suppose...??
    // I am still not very sure about this one
        // threshold
        // canny
        // and so on...
    // use bool to check whether a function processing fails or passes also 
    
    bool threshold(int val, int max_val, int type); // take the value from the threshold and apply it on the source and save to dest
    
    // -- to --  try to understand and implement
    bool canny_mask(int blur_size, int threshold, int ratio, int kernel_size); // apply effect on the source and save to dest
    // generic functions
    bool copy_to();
    bool scalar_mul();
    bool set_zeros();
    // -- ed --

    Vector2 get_image_size();

    // load_source - this will take a new image as source and clear the destination image
    bool load_source_from_path(String image);
    bool load_source(String image);

    // make_source - this function will make the final image the source image

    // get_source - to get the image from source
    Array get_source();
    
    Array get_image_data();
    

public:
    OpenCVServer();
    ~OpenCVServer();
};


#endif