#include "opencv.h"

bool OpenCV::load_image() {

    if (!image.empty()) {
        image.release();
    }

    // load the image at the path
    if (path_to_image.size() <= 0)
        path_to_image = String("/home/minraws/Projects/test/test.jpg");

    cv::String path(path_to_image.utf8().get_data());
    image = cv::imread( path, cv::IMREAD_UNCHANGED);

    cv::cvtColor(image, bw_image, cv::COLOR_BGR2GRAY);

    height = image.rows;   
    width = image.cols;   

    return !image.empty();
}  

Array OpenCV::get_image(bool get_processed_image) {   
    
    cv::Mat rgbFrame(width, height, CV_8UC3);
    
    // Be sure that we are dealing with RGB colorspace...
    if (!get_processed_image && !image.empty()) {
        cv::cvtColor(image, rgbFrame, CV_BGR2RGB);
    } else if (get_processed_image && !processed_image.empty()) {
        cv::cvtColor(processed_image, rgbFrame, CV_GRAY2RGB);
    } else {
        return Array();
    }

    // rows = height
    // column = width

    // returning a Array requires us to put less logic here for now to make my life easy at debugging parts of the code...
            // and this can be changed as per the need of the situation....
    // so Image.data is probably stored as CV_UC or something 
        // so the plan should be to use imencode to create a memory buffer
            // and use this memory buffer to create an array to use as ImageData
                // godot seemingly uses RGB8 or whatever we specify  -_- 

    Array arr;
    arr.resize(height*width*3);

    uchar* buffer = rgbFrame.data;

    for (int i = 0; i < height*width*3;i++)
        arr[i] = buffer[i];

    return arr;
}

void OpenCV::set_path_to_image(String _path) {
    path_to_image = _path;
    print_line("set_path_to_image");
}

String OpenCV::get_path_to_image() const {
    print_line("get_path_to_image");
    return "path_to_image";
}

void OpenCV::set_threshold_value(int _val) {
    cv::threshold(bw_image, processed_image, threshold_value, 100, 1);
    threshold_value = _val;
}

int OpenCV::get_threshold_value() const {
    return threshold_value;
}

void OpenCV::_bind_methods() {
    ClassDB::bind_method(D_METHOD("load_image"), &OpenCV::load_image);

    ClassDB::bind_method(D_METHOD("get_image", "get_processed_image"), &OpenCV::get_image, DEFVAL(false));

    ClassDB::bind_method(D_METHOD("set_path_to_image", "name"), &OpenCV::set_path_to_image);
    ClassDB::bind_method(D_METHOD("get_path_to_image"), &OpenCV::get_path_to_image);

    ClassDB::bind_method(D_METHOD("set_threshold_value", "threshold"), &OpenCV::set_threshold_value);
    ClassDB::bind_method(D_METHOD("get_threshold_value"), &OpenCV::get_threshold_value);


	ADD_PROPERTY(PropertyInfo(Variant::STRING, "path_to_image"), "set_path_to_image", "get_path_to_image");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "threshold_value"), "set_threshold_value", "get_threshold_value");
}

OpenCV::OpenCV() {
    // initialize the general variables here
    width = height = 0;
    threshold_value = 0;
}

OpenCV::~OpenCV() {
    // clean out the OpenCV stuff before killing yourself
    // although this is probably redundant with the C++ API of OpenCV
    if (!image.empty())
        image.release();
    if (!processed_image.empty())
        processed_image.release();
    print_line("destructor");
}