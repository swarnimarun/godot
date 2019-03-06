#include "opencv.h"

bool OpenCV::load_image() {
    // load the image at the location
    if (path_to_image.size() <= 0)
        path_to_image = String("/home/minraws/Projects/test/test.jpg");
    cv::String path(path_to_image.utf8().get_data());
    image = cv::imread( path, cv::IMREAD_UNCHANGED);    
    return !image.empty();
}  

void OpenCV::show_image() {
    cv::namedWindow( "Test Window", cv::WINDOW_AUTOSIZE );
    cv::imshow( "Test Window", image );
}

void OpenCV::set_path_to_image(Variant _path) {
    path_to_image = (String)_path;
}

Variant OpenCV::get_path_to_image() const {
    return (Variant)path_to_image;
}


void OpenCV::_bind_methods() {
    ClassDB::bind_method(D_METHOD("load_image"), &OpenCV::load_image);
    ClassDB::bind_method(D_METHOD("show_image"), &OpenCV::show_image);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "path_to_image", PROPERTY_HINT_TYPE_STRING, "PathToString"), "set_path_to_string", "get_path_to_string");
}

OpenCV::OpenCV() {

}

OpenCV::~OpenCV() {
    // clean out the OpenCV stuff before kill yourself
    if (!image.empty())
        image.release();
    // memdelete(path_to_image);
}