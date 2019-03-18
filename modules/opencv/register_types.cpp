
#include "register_types.h"
#include "opencv.h"
#include "opencv_server.h"

void register_opencv_types() {
	ClassDB::register_class<OpenCV>();
	ClassDB::register_class<OpenCVServer>();
}

void register_opencv_singletons() {
	Engine::get_singleton()->add_singleton(Engine::Singleton("OpenCVServer", OpenCVServer::get_singleton()));
}

void unregister_opencv_types() {
}
