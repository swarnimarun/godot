#include "opencv_server.h"

// moving header files here as I just found that it's better to not have
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"

#include "core/project_settings.h"

#include "core/variant.h"
#include "core/os/thread.h"


OpenCVProcess::OpenCVProcess() {
    // create a process point
}

void OpenCVProcess::set_process(int p_id) {
    process_id = p_id;   // this exists as a dummy value only
}

void OpenCVProcess::finished() {
    emit_signal("finish");
    this->call_deferred("free");
}

void OpenCVProcess::_bind_methods() {
    ClassDB::bind_method(D_METHOD("finished"), &OpenCVProcess::finished);
    ADD_SIGNAL(MethodInfo("finish"));
}

OpenCVServer::OpenCVServer() {
    thread = Thread::create(do_something, this);
    kill = false;
    changed = false;
}

OpenCVServer::~OpenCVServer() {
    if (!source.empty())
        source.release();
    if (!dest.empty())
        dest.release();
    if (!bw_img.empty())
        bw_img.release();

    kill = true;   

    // TODO: clean up all the existing process objs 

}

bool OpenCVServer::threshold(int val, int max_val, int type) {
    if (source.empty())
        return false;
    cv::threshold(bw_img, dest, val, max_val, type);
    dest_type = CV_BGR2RGB;

    emit_signal("value_update");    
    process = true;

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

    return true;
}
bool OpenCVServer::load_source_image(Image image) {
    cv::String path(ProjectSettings::get_singleton()->globalize_path(image.get_path()).utf8().get_data());
    source = cv::imread(path, cv::IMREAD_UNCHANGED);
    source_type = CV_BGR2RGB;

    if (source.empty())
        return false;

    cv::cvtColor(source, bw_img, cv::COLOR_BGR2RGB);

    height = source.rows;   
    width = source.cols;   

    return true;
}

PoolByteArray OpenCVServer::get_image_data() {       

    return image_data;
}

void OpenCVServer::process_image_data() {

    cv::Mat rgbFrame(width, height, CV_8UC3);
    cv::cvtColor(dest, rgbFrame, dest_type);

    Array arr;
    arr.resize(height*width*3);

    u_int8_t* buffer = rgbFrame.data;

    for (int i = 0; i < height*width*3;i++)
        arr[i] = buffer[i];

    Variant v = arr;

    image_data = PoolVector<u_int8_t>(v);
    emit_signal("processed_image");
}

Ref<ImageTexture> OpenCVServer::get_image_texture() {

    if (image_data.size <= 0) 
        return ;

    // 1. create a new Image from Image data
    Ref<Image> img = Object::cast_to<Image>(ClassDB::instance("Image"));
    img->create(width, height, false, Image::FORMAT_RGB8, image_data);         ///// THIS WORKS
    
    // 2. create ImageTexture from that Image
    Ref<ImageTexture> image_tex = Object::cast_to<ImageTexture>(ClassDB::instance("ImageTexture"));
    image_tex->create_from_image(img);
    return image_tex; 
}

void OpenCVServer::process_image() {
    process = true;
}


Ref<OpenCVProcess> OpenCVServer::create_process(int process_id) { // ability to create wrap and offload the process
    if (dest.empty()) {
        return Ref<OpenCVProcess>(); // useless
    }
    Ref<OpenCVProcess> ocvp;
    ocvp.instance();
	ocvp->set_process(process_id);
    // connect the signal from this to obj and then that directs control over other points
    process_image();
    ocvp->connect("processed_image", this, "finished");
    processes.push_back(ocvp);
    return ocvp;
}

void OpenCVServer::kill_me() {
    kill = true;
}

void OpenCVServer::do_something(void *data) {

    OpenCVServer *sev = (OpenCVServer *)data;

    while (true) {
        if (sev->process) {
            sev->process = false;  // this needs to be before so that we don't have any changes during processing which might get overriden.
            sev->process_image_data();
        }

        if (sev->kill)
            break;        
    }

    return;
}

Vector2 OpenCVServer::get_image_size() {
    return Vector2(width, height);
}

void OpenCVServer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("threshold", "value", "max_value", "type"), &OpenCVServer::threshold, DEFVAL(50), DEFVAL(100), DEFVAL(1));
    ClassDB::bind_method(D_METHOD("create_process", "process_id"), &OpenCVServer::create_process, DEFVAL(0));
    ClassDB::bind_method(D_METHOD("get_image_data"), &OpenCVServer::get_image_data);
    ClassDB::bind_method(D_METHOD("get_image_texture"), &OpenCVServer::get_image_texture);
    ClassDB::bind_method(D_METHOD("get_image_size"), &OpenCVServer::get_image_size);
    ClassDB::bind_method(D_METHOD("process_image"), &OpenCVServer::process_image);
    //ClassDB::bind_method(D_METHOD("kill_thread"), &OpenCVServer::kill_me);
    ClassDB::bind_method(D_METHOD("load_source_from_path", "source_image_path"), &OpenCVServer::load_source_from_path);
    ClassDB::bind_method(D_METHOD("load_source_image", "source_image"), &OpenCVServer::load_source_image);

    ADD_SIGNAL(MethodInfo("value_update"));
    ADD_SIGNAL(MethodInfo("processed_image"));
}