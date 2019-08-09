#include "wmf_stream.h"

#include "core/os/file_access.h"
#include "core/os/os.h"
#include "core/project_settings.h"


template<typename T>
void SafeRelease(T **ptr) {
    if (ptr && *ptr) {
        (*ptr)->Release();
    }
    return;
}

////////////////////////
/////Video_PLayback/////
////////////////////////

VideoStreamPlaybackWmf::VideoStreamPlaybackWmf() {
    HRESULT hr = S_OK;

    p_reader = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    // Initialize the COM library.
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    // if succeeded in the last operation
    if (SUCCEEDED(hr)) {
        // Initialize the Media Foundation platform.
        hr = MFStartup(MF_VERSION);
    }

    if (SUCCEEDED(hr)) {
        // Create the source reader to read the input file.
        // hr = MFCreateSourceReaderFromURL(wszSourceFile, NULL, &pReader);
        if (FAILED(hr)) {
            ERR_PRINT("Couldn't read from the source file properly.");
        }
    }
}
VideoStreamPlaybackWmf::~VideoStreamPlaybackWmf() {
    // delete_pointers();
    MFShutdown();
    CoUninitialize();
}

bool VideoStreamPlaybackWmf::open_file(const String &p_file) {
    String st = ProjectSettings::get_singleton()->globalize_path(p_file);
    print_line(st);
    return false;
}

void VideoStreamPlaybackWmf::stop() {
    if (playing) {
        // reset and clean stuff up

		delete_pointers();

		// pcm = NULL;

		// audio_frame = NULL;
		// video_frames = NULL;

		// video = NULL;
		// audio = NULL;

		// open_file(file_name); //Should not fail here...

		// video_frames_capacity = video_frames_pos = 0;
		// num_decoded_samples = 0;
		// samples_offset = -1;
		// video_frame_delay = video_pos = 0.0;
	}
	time = 0.0;
	playing = false;
}
void VideoStreamPlaybackWmf::play() {
	stop();

	delay_compensation = ProjectSettings::get_singleton()->get("audio/video_delay_compensation_ms");
	delay_compensation /= 1000.0;

	playing = true;
}

bool VideoStreamPlaybackWmf::is_playing() const {
    return playing;
}

void VideoStreamPlaybackWmf::set_paused(bool p_paused) {
    paused = p_paused;
}

bool VideoStreamPlaybackWmf::is_paused() const {
    return paused;
}

void VideoStreamPlaybackWmf::set_loop(bool p_enable) {
    // Loop
}

bool VideoStreamPlaybackWmf::has_loop() const {
    // check looping
    return false;
}

float VideoStreamPlaybackWmf::get_length() const {
    return 0.0;
}

float VideoStreamPlaybackWmf::get_playback_position() const {
    return video_pos;
}

void VideoStreamPlaybackWmf::seek(float p_time) {
    // TODO:
}

void VideoStreamPlaybackWmf::set_audio_track(int p_idx) {
    // TODO:
}

Ref<Texture> VideoStreamPlaybackWmf::get_texture() {
    return Ref<Texture>();
}

void VideoStreamPlaybackWmf::update(float p_delta) {
    // update the frame :D
}

void VideoStreamPlaybackWmf::set_mix_callback(AudioMixCallback p_callback, void *p_userdata) {}
int VideoStreamPlaybackWmf::get_channels() const { return 3; }
int VideoStreamPlaybackWmf::get_mix_rate() const { return 0; }

void VideoStreamPlaybackWmf::delete_pointers() {
    SafeRelease(&p_reader);
    SafeRelease(&byte_stream);
}

////////////////////////
//////Video_Stream//////
////////////////////////

VideoStreamWmf::VideoStreamWmf() {
    file = "";
    audio_track = -1;
}

Ref<VideoStreamPlayback> VideoStreamWmf::instance_playback() {
    Ref<VideoStreamPlaybackWmf> vw = memnew(VideoStreamPlaybackWmf);
    if (vw.is_valid() && vw->open_file(file))
        return vw;
    return NULL;
}

void VideoStreamWmf::set_audio_track(int p_track) {
}

void VideoStreamWmf::set_file(const String &p_file) {
    file = p_file;
}

String VideoStreamWmf::get_file() {
    return file;
}

void VideoStreamWmf::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_file", "file"), &VideoStreamWmf::set_file);
	ClassDB::bind_method(D_METHOD("get_file"), &VideoStreamWmf::get_file);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "file", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "set_file", "get_file");
}

////////////////////////
///////WMF_LOADER///////
////////////////////////

RES ResourceFormatLoaderWmf::load(const String &p_path, const String &p_original_path, Error *r_error) {

    // check if we have read permissions to the file
	FileAccess *f = FileAccess::open(p_path, FileAccess::READ);
	if (!f) {
		if (r_error) {
			*r_error = ERR_CANT_OPEN;
		}
		return RES();
	}

    // create the new stream
	Ref<VideoStreamWmf> wmf_stream;
    wmf_stream.instance();

    if (wmf_stream.is_null())
        return RES();

	wmf_stream->set_file(p_path);

    // ! For testing
    wmf_stream->instance_playback();

    // check for propagated errors and clear them if any
	if (r_error) {
		*r_error = OK;
	}

	return wmf_stream;
}

void ResourceFormatLoaderWmf::get_recognized_extensions(List<String> *p_extensions) const {

	p_extensions->push_back("mp4");
}

bool ResourceFormatLoaderWmf::handles_type(const String &p_type) const {

	return ClassDB::is_parent_class(p_type, "VideoStream");
}

String ResourceFormatLoaderWmf::get_resource_type(const String &p_path) const {

	String el = p_path.get_extension().to_lower();
	if (el == "mp4")
		return "VideoStreamWmf";
	return "";
}
