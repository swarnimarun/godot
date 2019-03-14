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
    cv::Mat bw_image;
    cv::Mat processed_image;
    cv::Mat detected_edges;
    cv::Mat dst;

    int height;
    int width;
public:
    // some variables to be accessible from the Engine side
    String path_to_image;

    Vector2 size;
    
    int threshold_value;

    int edgeThresh = 1;
    int lowThreshold;
    int const max_lowThreshold = 100;
    int ratio = 3;
    int kernel_size = 3;

protected:
	static void _bind_methods(); // bind the public methods to the class db

public:
    bool load_image();
    Array get_image(bool get_processed_image);
    
    void set_threshold_value(int _val);
    int get_threshold_value() const;

    void set_path_to_image(String _path);
    String get_path_to_image() const;

    void set_image_size(Vector2 _value);
    Vector2 get_image_size() const;

    void canny_edge();

    OpenCV();
    ~OpenCV();
};



#endif