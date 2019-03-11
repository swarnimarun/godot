#ifndef OPENCV_H
#define OPENCV_H

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"

#include "core/image.h"
#include "core/resource.h"
#include "core/reference.h"
#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "editor/property_editor.h"


// dummy class to act as an abstraction for the opencv library and godot

// as a helping reminder everything without the cv:: is from the Godot libs 
//                                        make sure you take care of it properly


class OpenCV : public Resource {
    GDCLASS(OpenCV, Resource)

private:
    // some variables for the OpenCV class
    cv::Mat image;

    cv::Mat grey_img;
    cv::Mat dst;

    cv::String window_name;

public:
    // some variables to be accessible from the Engine side
    String path_to_image;

    int height;
    int width;

    // default; wait for key press
    int millisecs;

    int threshold_value;

protected:
	static void _bind_methods(); // bind the public methods to the class db

public:
    bool load_image();
    void show_window();
    void update_window();
    void close_window();
    Array get_image() const;

    void set_threshold();
    
    void set_threshold_value(int _val);
    int get_threshold_value() const;

    void set_path_to_image(String _path);
    String get_path_to_image() const;

    OpenCV();
    ~OpenCV();
};



#endif