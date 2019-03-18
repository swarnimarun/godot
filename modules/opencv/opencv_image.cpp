#include "opencv_image.hpp"
#include "core/project_settings.h"

// constructor
OpenCVImage::OpenCVImage() {

}

// destructor
OpenCVImage::~OpenCVImage() {
    
}

void OpenCVImage::set_image(bool relative, String path) {
    if (relative) {
        relative_path = path;
        absolute_path = ProjectSettings::get_singleton()->globalize_path(path);
    }
    absolute_path = path;
}

String OpenCVImage::get_address(bool relative) const{
    if (relative)
        return relative_path;
    return absolute_path;
}

void OpenCVImage::_bind_methods() {
    // bind the methods
    ClassDB::bind_method(D_METHOD("set_image", "relative", "path"), &OpenCVImage::set_image);
    ClassDB::bind_method(D_METHOD("get_address", "relative"), &OpenCVImage::get_address);

}