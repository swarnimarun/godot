#include "opencv.h"

bool OpenCV::load_image() {
    // load the image at the location
    // cv::String path("test.jpg");
    image = cvLoadImageM( "placeholder.jpg", CV_LOAD_IMAGE_UNCHANGED );
    return false;
}  

void OpenCV::show_image() {
    cvNamedWindow("My Window", CV_WINDOW_AUTOSIZE);
    cvShowImage("My Window", image);
    cvWaitKey(0);
}

void OpenCV::_bind_methods() {
    ClassDB::bind_method(D_METHOD("load_image"), &OpenCV::load_image);
    ClassDB::bind_method(D_METHOD("show_image"), &OpenCV::show_image);
}

OpenCV::OpenCV() {

}

OpenCV::~OpenCV() {
    // clean out the OpenCV stuff before kill yourself
    free(image);
}