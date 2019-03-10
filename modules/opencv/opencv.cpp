#include "opencv.h"

bool OpenCV::load_image() {
    // load the image at the path
    if (path_to_image.size() <= 0)
        path_to_image = String("/home/minraws/Projects/test/test.jpg");
    cv::String path(path_to_image.utf8().get_data());
    image = cv::imread( path, cv::IMREAD_UNCHANGED);
    cv::cvtColor(image, grey_img, cv::COLOR_BGR2GRAY);
    height = image.rows;   
    width = image.cols;   
    return !image.empty();
}  

void OpenCV::show_window() {
    cv::namedWindow( window_name, cv::WINDOW_AUTOSIZE );
    if (!dst.empty())
        cv::imshow( window_name, dst );
    else
        cv::imshow( window_name, image );
    cv::waitKey(10); // transfer context to godot as soon as possible
}

PoolByteArray OpenCV::get_image() const {
    
    // use this pointer data to get all the rows and columns from OpenCV
    // rows = height
    // column = width

    // returning a PoolByteArray requires us to put less logic here for now to make my life easy at debugging parts of the code...
            // and this can be changed as per the need of the situation....
    // so Image.data is probably stored as CV_UC1 or something 
        // so the plan should be to use imencode to create a memory buffer
            // and use this memory buffer to create an array to use as ImageData
                // godot seemingly uses RGB8 or whatever we specify  -_- 

    PoolByteArray arr;
    arr.resize(height*width*3);

    std::vector<uchar> buffer;
    cv::imencode( ".jpg", image, buffer);

    uchar* data = &buffer[0];  // hopefully everything is stored contigously
    memcpy( &arr, data, height*width*3 ); // trying to knock myself out here... :P

    // print_line( String::num_uint64(width) );
    return arr;

    // untested code.... :((
}

void OpenCV::close_window() {
    cv::destroyWindow(window_name);
}

void OpenCV::set_path_to_image(String _path) {
    path_to_image = _path;
    print_line("set_path_to_image");
}

String OpenCV::get_path_to_image() const {
    print_line("get_path_to_image");
    return "path_to_image";
}

void OpenCV::update_window() {
    cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);
    cv::imshow(window_name, dst);
    cv::waitKey(10);
}

void OpenCV::set_threshold() {
    cv::threshold(grey_img, dst, threshold_value, 255, 3);
    close_window();
    update_window();
}

void OpenCV::set_threshold_value(int _val) {
    threshold_value = _val;
}

int OpenCV::get_threshold_value() const {
    return threshold_value;
}

void OpenCV::_bind_methods() {
    ClassDB::bind_method(D_METHOD("load_image"), &OpenCV::load_image);
    ClassDB::bind_method(D_METHOD("show_window"), &OpenCV::show_window);
    ClassDB::bind_method(D_METHOD("close_window"), &OpenCV::close_window);

    ClassDB::bind_method(D_METHOD("get_image"), &OpenCV::get_image);

    ClassDB::bind_method(D_METHOD("set_path_to_image", "name"), &OpenCV::set_path_to_image);
    ClassDB::bind_method(D_METHOD("get_path_to_image"), &OpenCV::get_path_to_image);

    ClassDB::bind_method(D_METHOD("set_threshold_value", "threshold"), &OpenCV::set_threshold_value);
    ClassDB::bind_method(D_METHOD("get_threshold_value"), &OpenCV::get_threshold_value);

    ClassDB::bind_method(D_METHOD("set_threshold"), &OpenCV::set_threshold);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "path_to_image"), "set_path_to_image", "get_path_to_image");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "threshold_value"), "set_threshold_value", "get_threshold_value");
}

OpenCV::OpenCV() {
    window_name = cv::String("Test Window");
}

OpenCV::~OpenCV() {
    // clean out the OpenCV stuff before kill yourself
    if (!image.empty())
        image.release();
    print_line("destructor");
    // memdelete(path_to_image);
}