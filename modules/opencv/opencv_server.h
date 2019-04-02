
#ifndef OPENCV_SERVER_H
#define OPENCV_SERVER_H

#include "opencv2/core.hpp"

#include "core/object.h"
#include "core/os/thread.h"

#include "scene/resources/texture.h"


// Quick wrapper for Signals as object 
class OpenCVProcess : public Reference {
	GDCLASS(OpenCVProcess, Reference);

    int process_id;
    bool paused;

protected:
	static void _bind_methods();

public:
	void set_process(int p_id);
	float get_process() const;

    void finished();

	void set_pause_mode_process(bool p_pause_mode_process);
	bool is_pause_mode_process();

	OpenCVProcess();
};

class OpenCVServer : public Object {
    GDCLASS(OpenCVServer, Object)

protected:
	static void _bind_methods(); // bind the public methods to the class db

private:
    // source image
    cv::Mat source;
    int source_type;

    // destination/final image
    cv::Mat dest;
    int dest_type;
    
    // black and white image
    cv::Mat bw_img;

    int height;
    int width;

    int task;
    
    //PoolVector<u_char> image_data;
    Ref<ImageTexture> image_tex;

    bool kill;
    bool process;
    
    bool changed;
	
    // TODO: Implement a process queue
    // List<Array> process_queue; // Array should hold the arguments for the process it wants to call starting from process ID

    Thread *thread;

    void process_image();

    PoolByteArray get_image_data();
    void process_image_tex();


public:

    enum Process{
        OPENCV_PROCESS_GRAYSCALE = 0,
        OPENCV_PROCESS_THRESHOLD = 1,
        OPENCV_PROCESS_CANNY = 2
    };

    // My simple fun
    List<Ref<OpenCVProcess>> processes;
    
    Ref<OpenCVProcess> start_process(Array p_proc); // ability to create wrap and offload the process
    
    static void do_something(void *data);  // this is the main function n

    // operations will be a function that can be called as per the enum that we have sorta like notifications I suppose...??
    // I am still not very sure about this one
        // threshold
        // canny
        // and so on...
    // use bool to check whether a function processing fails or passes also 
    
    bool threshold(int val, int max_val, int type); // take the value from the threshold and apply it on the source and save to dest
    bool grayscale(); // simply make the image gray scale
    
    // -- to --  try to understand and implement
    bool canny_mask(int blur_size, int threshold, int ratio, int kernel_size); // apply effect on the source and save to dest
    // generic functions
    bool copy_to();
    bool scalar_mul();
    bool set_zeros();
    // -- ed --


    // load_source - this will take a new image as source and clear the destination image
    bool load_source_from_path(String image);

    
    Ref<ImageTexture> get_image_texture();
    void cleanup();

public:
    OpenCVServer();
    ~OpenCVServer();
};


#endif