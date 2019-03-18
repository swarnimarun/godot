
#include "register_types.h"
#include "opencv.h"
#include "opencv_server.h"

void register_opencv_types() {
	ClassDB::register_class<OpenCV>();
	ClassDB::register_class<OpenCVServer>();
}

void register_opencv_singletons() {

}

void unregister_opencv_types() {
}
