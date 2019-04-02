#include "opencv_server.h"

// moving header files here as I just found that it's better to not have
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"

#include "core/project_settings.h"

#include "core/variant.h"
#include "core/os/thread.h"


////////// OPENCV PROCESS ///////// 

OpenCVProcess::OpenCVProcess() {
    // create a process point
}

void OpenCVProcess::set_process(int p_id) {
    process_id = p_id;   // this exists as a dummy value only
    // this might come in handy for future use
}

void OpenCVProcess::finished() {
    emit_signal("finish");
    //this->call_deferred("free");
    //this->unreference();
}

void OpenCVProcess::_bind_methods() {
    ClassDB::bind_method(D_METHOD("finished"), &OpenCVProcess::finished);
    ADD_SIGNAL(MethodInfo("finish"));
}


////////// OPENCV SERVER //////////////


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

    image_tex.unref();

    // clean up all the existing process objs 
    for (auto E = processes.front(); E; E->next()) {
        E->get()->unreference(); // ensure they have been unref'd
    }
    processes.clear();

}

////////// GET FUNCTIONS /////////////


Ref<ImageTexture> OpenCVServer::get_image_texture() {
    return image_tex; 
}


////////// PROCESSES ///////////

bool OpenCVServer::threshold(int val, int max_val, int type) {
    
    if (source.empty())
        return false;
    
    cv::threshold(bw_img, dest, val, max_val, type);
    dest_type = CV_GRAY2RGB;

    emit_signal("value_update");    
    
    return true;
}

bool OpenCVServer::grayscale() {
    
    if (source.empty())
        return false;
    
    dest = bw_img;
    dest_type = CV_GRAY2RGB;

    emit_signal("value_update");    
    
    return true;
}

///// LOAD ///////

bool OpenCVServer::load_source_from_path(String image) {
    cv::String path(image.utf8().get_data());
    source = cv::imread(path, cv::IMREAD_UNCHANGED);
    source_type = CV_BGR2RGB;

    if (source.empty())
        return false;

    cv::cvtColor(source, bw_img, cv::COLOR_BGR2GRAY);

    height = source.rows;
    width = source.cols;   

    return true;
}


//////// INTERNALS //////////////

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



void OpenCVServer::process_image_tex() {
    // 1. create a new Image from Image data
    Ref<Image> img = Object::cast_to<Image>(ClassDB::instance("Image"));
    img->create(width, height, false, Image::FORMAT_RGB8, get_image_data());         ///// THIS WORKS
    
    // 2. create ImageTexture from that Image
    Ref<ImageTexture> img_tex = Object::cast_to<ImageTexture>(ClassDB::instance("ImageTexture"));
    img_tex->create_from_image(img);
    image_tex = img_tex;
    emit_signal("processed_image");
}




Ref<OpenCVProcess> OpenCVServer::start_process(Array p_proc) { // ability to create wrap and offload the process
    
    if (process || p_proc.size() < 1)
        return Ref<OpenCVProcess>(); // basically returning NULL


    int process_id = int(p_proc[0]);

    Ref<OpenCVProcess> ocvp;
    ocvp.instance();
	ocvp->set_process(process_id);
    // connect the signal from this to obj and then that directs control over other points
    
    switch (process_id) {
        case OPENCV_PROCESS_GRAYSCALE:   {
                // Get Grayscale Image call
                if (!grayscale()) {
                    _err_print_error(FUNCTION_STR, __FILE__, __LINE__, "Method/Function Failed.");
                    return Ref<OpenCVProcess>();
                }
            }
            break;
        case OPENCV_PROCESS_THRESHOLD:   {
                // Threshold process call
                bool value = threshold(int(p_proc[1]), int(p_proc[2]), int(p_proc[3]));
                if (p_proc.size() < 3  || !value){
                    _err_print_error(FUNCTION_STR, __FILE__, __LINE__, "Method/Function Failed.");
                    return Ref<OpenCVProcess>();
                }
            }
            break;
        case OPENCV_PROCESS_CANNY:
            return Ref<OpenCVProcess>();
            break;
        default:
            return Ref<OpenCVProcess>();
            break;
    }

    process = true;

    this->connect("processed_image", ocvp.ptr(), "finished");
    processes.push_back(ocvp);
    return ocvp;
}


void OpenCVServer::do_something(void *data) {

    OpenCVServer *sev = (OpenCVServer *)data;

    while (true) {
        if (sev->process) {
            sev->process_image_tex();
            // once this method is called the thread is blocked and because we aren't stacking calls so it will only be done once.

            sev->process = false;  // this needs to be before so that we don't have any changes during processing which might get overriden.
        }

        if (sev->kill)
            break;        
    }

    return;
}




void OpenCVServer::_bind_methods() {
    //ClassDB::bind_method(D_METHOD("threshold", "value", "max_value", "type"), &OpenCVServer::threshold, DEFVAL(50), DEFVAL(100), DEFVAL(1));
    ClassDB::bind_method(D_METHOD("start_process", "process_arguments"), &OpenCVServer::start_process);
    //ClassDB::bind_method(D_METHOD("get_image_data"), &OpenCVServer::get_image_data);
    ClassDB::bind_method(D_METHOD("get_image_texture"), &OpenCVServer::get_image_texture);
    
    ClassDB::bind_method(D_METHOD("load_source_from_path", "source_image_path"), &OpenCVServer::load_source_from_path);
    //ClassDB::bind_method(D_METHOD("load_source_image", "source_image"), &OpenCVServer::load_source_image);

    ADD_SIGNAL(MethodInfo("value_update"));
    ADD_SIGNAL(MethodInfo("processed_image"));
    
}