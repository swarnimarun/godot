#include "opencv_server.h"

// moving header files here as I just found that it's better to not have
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"

#include "core/variant.h"

OpenCVServer::OpenCVServer() {
    //thread = Thread::create(do_something, this);
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

bool OpenCVServer::load_source_from_path(String image) {
    cv::String path(image.utf8().get_data());
    source = cv::imread(path, cv::IMREAD_UNCHANGED);
    source_type = CV_BGR2RGB;

    if (source.empty())
        return false;

    cv::cvtColor(source, bw_img, cv::COLOR_BGR2RGB);

    height = source.rows;   
    width = source.cols;   

    //image_data->resize(height*width*3);

    return true;
}

PoolByteArray OpenCVServer::get_image_data() {       
    
    cv::Mat rgbFrame(width, height, CV_8UC3);
    cv::cvtColor(dest, rgbFrame, dest_type);
   
    Array arr;
    arr.resize(height*width*3);

    u_int8_t* buffer = rgbFrame.data;

    for (int i = 0; i < height*width*3;i++)
        arr[i] = buffer[i];

    Variant v = arr;

    return PoolVector<u_int8_t>(v);
}

Ref<ImageTexture> OpenCVServer::get_image_texture() {

    // 1. create a new Image from Image data
    Ref<Image> img = Object::cast_to<Image>(ClassDB::instance("Image"));
    img->create(width, height, false, Image::FORMAT_RGB8, get_image_data());         ///// THIS WORKS
    
    // 2. create ImageTexture from that Image
    Ref<ImageTexture> image_tex = Object::cast_to<ImageTexture>(ClassDB::instance("ImageTexture"));
    image_tex->create_from_image(img);
    return image_tex; 
}

// void OpenCVServer::do_something(void *data) {

    

// }

Vector2 OpenCVServer::get_image_size() {
    return Vector2(width, height);
}

void OpenCVServer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("threshold", "value", "max_value", "type"), &OpenCVServer::threshold, DEFVAL(50), DEFVAL(100), DEFVAL(1));
    ClassDB::bind_method(D_METHOD("get_image_data"), &OpenCVServer::get_image_data);
    ClassDB::bind_method(D_METHOD("get_image_texture"), &OpenCVServer::get_image_texture);
    ClassDB::bind_method(D_METHOD("get_image_size"), &OpenCVServer::get_image_size);
    ClassDB::bind_method(D_METHOD("load_source_from_path", "source_image_path"), &OpenCVServer::load_source_from_path);

    ADD_SIGNAL(MethodInfo("value_update"));
}