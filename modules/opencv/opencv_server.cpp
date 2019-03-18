#include "opencv_server.h"

// moving header files here as I just found that it's better to not have
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"

#include "core/os/thread.h"

OpenCVServer::OpenCVServer() {

}

OpenCVServer::~OpenCVServer() {
    if (!source.empty())
        source.release();
    if (!dest.empty())
        dest.release();
    if (!bw_img.empty())
        bw_img.release();
}

bool OpenCVServer::threshold(int val, int max_val, int type) {
    if (source.empty())
        return false;
    cv::threshold(bw_img, dest, val, max_val, type);
    dest_type = CV_BGR2RGB;
    emit_signal("value_update");
    return true;
}

bool OpenCVServer::load_source(String image) {
    cv::String path(image.utf8().get_data());
    source = cv::imread(path, cv::IMREAD_UNCHANGED);
    source_type = CV_BGR2RGB;

    if (source.empty())
        return false;

    cv::cvtColor(source, bw_img, cv::COLOR_BGR2RGB);

    height = source.rows;   
    width = source.cols;   

    return true;
}

Array OpenCVServer::get_image_data() {       
    cv::Mat rgbFrame(width, height, CV_8UC3);
    cv::cvtColor(dest, rgbFrame, dest_type);
   
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

    u_int8_t* buffer = rgbFrame.data;

    for (int i = 0; i < height*width*3;i++)
        arr[i] = buffer[i];

    return arr;
}

// Ref<Image> OpenCVServer::get_image() {
//     cv::Mat rgbFrame(width, height, CV_8UC3);
//     cv::cvtColor(dest, rgbFrame, dest_type);
   
//     // rows = height
//     // column = width

//     // returning a Array requires us to put less logic here for now to make my life easy at debugging parts of the code...
//             // and this can be changed as per the need of the situation....
//     // so Image.data is probably stored as CV_UC or something 
//         // so the plan should be to use imencode to create a memory buffer
//             // and use this memory buffer to create an array to use as ImageData
//                 // godot seemingly uses RGB8 or whatever we specify  -_- 

//     PoolByteArray data;
//     data.resize(height*width*3);
//     uchar* buffer = rgbFrame.data;
//     print_line("Start appending");

//     for (int i = 0; i < height*width*3;i++)
//         data.set(i, buffer[i]);
    
//     //memcpy(&data, &arr, data.size());
//     print_line("Ok so appending works");
    
//     Ref<Image> img;
//     img->create(width, height, false, Image::FORMAT_RGB8); // so this refuses to work for some reason.... :(

//     print_line("So creating image works too.... WTF");

//     return img;
// }

Vector2 OpenCVServer::get_image_size() {
    return Vector2(width, height);
}

void OpenCVServer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("threshold", "value", "max_value", "type"), &OpenCVServer::threshold, DEFVAL(50), DEFVAL(100), DEFVAL(1));
    ClassDB::bind_method(D_METHOD("get_image_data"), &OpenCVServer::get_image_data);
    // ClassDB::bind_method(D_METHOD("get_image"), &OpenCVServer::get_image);
    ClassDB::bind_method(D_METHOD("get_image_size"), &OpenCVServer::get_image_size);
    ClassDB::bind_method(D_METHOD("load_source", "source_image_path"), &OpenCVServer::load_source);

    ADD_SIGNAL(MethodInfo("value_update"));
}