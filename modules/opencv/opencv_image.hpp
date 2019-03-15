
#ifndef OPENCV_IMAGE_H
#define OPENCV_IMAGE_H

#include "core/resource.h"
#include "core/project_settings.h"

// this is just a plain data holder to 

class OpenCVImage : public Resource {
    GDCLASS(OpenCVImage, Resource)

private:
    bool is_editor;
    String relative_path; // store the res:// path to the image
    String absolute_path; // store the absolute path to the image
protected:
	static void _bind_methods(); // bind the public methods to the class db
public:
    bool isEqual(OpenCVImage image) const; // check if the image resource is a duplicate
    bool isEqualString(String image_path, bool is_absolute) const; // check if the path is equal

    void set_image(String path, bool relative);

    String get_address(bool relative) const; // this will give the absolute path by default 

    OpenCVImage();
    ~OpenCVImage();
};


#endif