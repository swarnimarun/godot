

#ifndef OPENCV_H
#define OPENCV_H

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#include "core/image.h"
#include "core/resource.h"
#include "core/reference.h"
#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "editor/property_editor.h"


// dummy class to act as an abstraction for the opencv library and godot

class OpenCV : public Resource {
    GDCLASS(OpenCV, Resource)

private:
    // some variables for the OpenCV class
public:
    // some variables to be accessible from the Engine side

};



#endif