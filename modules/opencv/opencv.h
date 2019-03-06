#ifndef OPENCV_H
#define OPENCV_H

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

#include "core/image.h"
#include "core/resource.h"
#include "core/reference.h"
#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "editor/property_editor.h"


// dummy class to act as an abstraction for the opencv library and godot

// as a helping reminder everything without the cv is from the Godot libs 
//                                        make sure you take care of it properly


class OpenCV : public Resource {
    GDCLASS(OpenCV, Resource)

private:
    // some variables for the OpenCV class
    CvMat* image;
public:
    // some variables to be accessible from the Engine side
    String path_to_image;

protected:
	static void _bind_methods(); // bind the public methods to the class db

public:
    bool load_image();
    void show_image();

    OpenCV();
    ~OpenCV();
};



#endif