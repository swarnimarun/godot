#include "opencv_image.hpp"

// constructor
OpenCVImage::OpenCVImage() {

}

// destructor
OpenCVImage::~OpenCVImage() {
    
}

void OpenCVImage::set_image(String path, bool relative) {
    absolute_path = ProjectSettings::globalize_path(path); // fix IT
}

String OpenCVImage::get_address(bool relative) const{
    if (relative)
        return relative_path;
    return absolute_path;
}
void OpenCVImage::_bind_methods() {
    // bind the methods
}