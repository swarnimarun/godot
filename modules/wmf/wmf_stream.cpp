#include "wmf_stream.h"

#include "core/os/file_access.h"
#include "core/os/os.h"
#include "core/project_settings.h"

#include "propvarutil.h"

// helper method to get the flags of the media
HRESULT WmfMediaSource::_get_source_flags(ULONG *pulFlags) {
	ULONG flags = 0;

	PROPVARIANT var;
	PropVariantInit(&var);

	HRESULT hr = reader->GetPresentationAttribute(
		MF_SOURCE_READER_MEDIASOURCE, 
		MF_SOURCE_READER_MEDIASOURCE_CHARACTERISTICS, 
		&var);

	if (SUCCEEDED(hr)) {
		hr = PropVariantToUInt32(var, &flags);
	}
	if (SUCCEEDED(hr)) {
		*pulFlags = flags;
	}

	PropVariantClear(&var);
	return hr;
}


// check to see if the media is seekable
BOOL WmfMediaSource::_source_can_seek() {
	BOOL bCanSeek = FALSE;
	ULONG flags;
	if (SUCCEEDED(_get_source_flags(&flags))) {
		bCanSeek = ((flags & MFMEDIASOURCE_CAN_SEEK) == MFMEDIASOURCE_CAN_SEEK);
	}
	return bCanSeek;
}

// TODO: Expose it
HRESULT WmfMediaSource::_set_position(const LONGLONG& hnsPosition) {
	PROPVARIANT var;
	HRESULT hr = InitPropVariantFromInt64(hnsPosition, &var);
	if (SUCCEEDED(hr)) {
		// GUID_NULL is for 100 nanosec unit
		hr = reader->SetCurrentPosition(GUID_NULL, var);
		PropVariantClear(&var);
	}
	return hr;
}

// method to get the source reader duration in 100 nanosec units
HRESULT WmfMediaSource::_get_duration(LONGLONG *phnsDuration) {
	PROPVARIANT var;
	HRESULT hr = reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, 
		MF_PD_DURATION, &var);
	if (SUCCEEDED(hr)) {
		hr = PropVariantToInt64(var, phnsDuration);
		PropVariantClear(&var);
	}
	return hr;
}

void WmfMediaSource::_process_width_height() {
	IMFMediaType *p_type;
	reader->GetCurrentMediaType(0, &p_type); // I am guessing stream 0 is the video stream if it's not well atleast no one died
	MFGetAttributeSize(p_type, MF_MT_FRAME_SIZE, &width, &height);
	width = 854;
	height = 480;
}

WmfMediaSource::WmfMediaSource() {
	ended = false;
	reader = NULL;
	// Initialize the Media Foundation platform.
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	// if succeeded in the last operation
	if (SUCCEEDED(hr)) {
		hr = MFStartup(MF_VERSION);
	}

	frame_read = false;
	current_frame_count = 0;
}

WmfMediaSource::WmfMediaSource(const String &path, short type_flags) {
	reader = NULL;
	ended = false;
	// Initialize the COM library.
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	frame_read = true;
	// if succeeded in the last operation
	if (SUCCEEDED(hr)) {
		// Initialize the Media Foundation platform.
		hr = MFStartup(MF_VERSION);
		if (SUCCEEDED(hr)) {
			// Create the source reader to read the input file.
			if (create_source(path)) {
				set_media_output_type(type_flags);
				frame_read = false;
				current_frame_count = 0;
			}
		}
	}
	_process_width_height();
}

WmfMediaSource::~WmfMediaSource() {
	ended = true;
	if (reader)
		reader->Release();
	if (sample)
		sample->Release();
	if (outputType)
		outputType->Release();
	MFShutdown();
	CoUninitialize();
}

bool WmfMediaSource::create_source(const String &path) {
	const wchar_t *path_wchar = path.c_str(); // CharType is wchar_t
	// plan some IMFAttributes that might need to be added
	reader = NULL;
	ended = false;
	frame_read = true;

	HRESULT hr = MFCreateSourceReaderFromURL(path.c_str(), NULL, &reader);
	if (SUCCEEDED(hr)) {
		frame_read = false;
		current_frame_count = 0;
		_process_width_height();
		set_media_output_type(0);
		return true;
	}
	return false;
}

long WmfMediaSource::get_length() {
	if (!reader)
		return 0;
	LONGLONG val;
	_get_duration(&val);
	return val;
}

unsigned int WmfMediaSource::get_width() {
	if (!reader)
		return 0;
	return width;
}

unsigned int WmfMediaSource::get_height() {
	if (!reader)
		return 0;
	return height;
}

bool WmfMediaSource::get_frame_data(PoolVector<uint8_t> &bytevec) {
	// return the current frame if it's not been read
	if (!frame_read) {
		PoolVector<uint8_t>::Write wrt = bytevec.write();
		uint8_t *w = wrt.ptr();
		// create a IMFMediaBuffer using the ConvertToContigousBuffer
		IMFMediaBuffer *media_buf;
		HRESULT hr = sample->ConvertToContiguousBuffer(&media_buf);
		if (SUCCEEDED(hr)) {
			// convert to a IMF2DBuffer using the QueryInterface function on MediaBuffer
			BYTE *buffer;
			DWORD CURR_SIZE;
			IMF2DBuffer *d_buff;
			hr = media_buf->QueryInterface<IMF2DBuffer>(&d_buff);
			if (SUCCEEDED(hr)) {
				// use the Lock2D if the isContigous is true
				// otherwise use Lock which is slower but guarantees contigous memory 
				LONG pitch;
				hr = d_buff->Lock2D(&buffer, &pitch);
				for (int j = 0; j < 480; j++) {
					for (int i = 0; i < 854; i++) {
						*w++ = *(buffer + pitch * j);
						*w++ = *(buffer + 1 + pitch * j);
						*w++ = *(buffer + 2 + pitch * j);
						*w++ = 255;
					}
				}
				hr = d_buff->Unlock2D();
				// frame_read = true;
				return true;
			} else if (SUCCEEDED(media_buf->Lock(&buffer, NULL, &CURR_SIZE))) {
				if (CURR_SIZE == (854 * 3 * 480))
					printf("it fits yay!!");
				for (int j = 0; j < 480; j++) {
					for (int i = 0; i < 854; i++) {
						*w++ = *(buffer + i);
						*w++ = *(buffer + 1 + i);
						*w++ = *(buffer + 2 + i);
						*w++ = 255;
					}
				}
				print_line("step 4");				
				media_buf->Unlock();
				return true;
			}
		}
	}
	PoolVector<uint8_t>::Write wrt = bytevec.write();
	uint8_t *w = wrt.ptr();
	for (int j = 0; j < 480; j++) {
		for (int i = 0; i < 854; i++) {
			// MAGENTA LIKE COLOR
			*w++ = 220;
			*w++ = 50;
			*w++ = 220;
			*w++ = 255;
		}
	}
	return true;
}

bool WmfMediaSource::move_frame(float time = 0, bool forward = true) {
	// move forward by a minimum the provided time
	// get the next best frame after the duration
	// use the loop with the sample duration

	DWORD flags;
	LONGLONG timestamp;

	HRESULT hr = reader->ReadSample(
		MF_SOURCE_READER_FIRST_VIDEO_STREAM,
		0, &actualVideoStreamIndex, &flags, &timestamp, &sample);

	if (FAILED(hr)) {
		return false;
	}

	if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
		ended = true;
	}

	current_frame_count++;
	frame_read = false;
	return true;
}

long long WmfMediaSource::get_frame_pos() const {
	// current frame count
	return current_frame_count;
}

float WmfMediaSource::get_frame_time() const {
	LONGLONG time;
	if (SUCCEEDED(sample->GetSampleTime(&time)))
		return time / 1e9; // convert from nanosec to sec
	return 0;
}

bool WmfMediaSource::has_ended() const {
	return ended;
}

bool WmfMediaSource::set_media_output_type(short type) {
	// change the media output type
	HRESULT hr = MFCreateMediaType(&outputType);
	if (SUCCEEDED(hr)) {
		hr = outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		if (SUCCEEDED(hr)) {
			hr = outputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB8);
			reader->SetCurrentMediaType(0, NULL, outputType);
			return true;		
		}
	}
	return false;
}

////////////////////////
/////Video_PLayback/////
////////////////////////

VideoStreamPlaybackWmf::VideoStreamPlaybackWmf() : texture(memnew(ImageTexture)) {
    source = new WmfMediaSource();
}
VideoStreamPlaybackWmf::~VideoStreamPlaybackWmf() {
	if (source)
		free(source);
}

bool VideoStreamPlaybackWmf::open_file(const String &p_file) {
    String st = ProjectSettings::get_singleton()->globalize_path(p_file);
	if (source->create_source(st)) {
		frame_data.resize((source->get_width() * source->get_height()) * 4); // 4 values r,g,b,a
		texture->create(source->get_width(), source->get_height(), Image::FORMAT_RGBA8, Texture::FLAG_FILTER | Texture::FLAG_VIDEO_SURFACE);
		return true;
	}
	return false;
}

void VideoStreamPlaybackWmf::stop() {
    if (playing) {
        // reset and clean stuff up

		// delete_pointers();

		// pcm = NULL;

		// audio_frame = NULL;
		// video_frames = NULL;

		// frame_data.resize(0);

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
	paused = false;
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
    return 50.0;
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
	return texture;
}

void VideoStreamPlaybackWmf::update(float p_delta) {
    // update the frame :D
    if ((!playing || paused) || !source)
		return;

	time += p_delta;

	if (time < video_pos) {
		return;
	}

	// TODO: get the audio stream working using Media Foundation itself
	// Most of this uses the features provided by Media Foundation itself like moving across frames and caching to some level
	// we can just reuse to not have to rewrite much

	// bool audio_buffer_full = false;

	// if (samples_offset > -1) {

	// 	//Mix remaining samples
	// 	const int to_read = num_decoded_samples - samples_offset;
	// 	const int mixed = mix_callback(mix_udata, pcm + samples_offset * webm->getChannels(), to_read);
	// 	if (mixed != to_read) {

	// 		samples_offset += mixed;
	// 		audio_buffer_full = true;
	// 	} else {

	// 		samples_offset = -1;
	// 	}
	// }

	// const bool hasAudio = (audio && mix_callback);
	// while ((hasAudio && !audio_buffer_full && !has_enough_video_frames()) ||
	// 		(!hasAudio && video_frames_pos == 0)) {

	// 	if (hasAudio && !audio_buffer_full && audio_frame->isValid() &&
	// 			audio->getPCMF(*audio_frame, pcm, num_decoded_samples) && num_decoded_samples > 0) {

	// 		const int mixed = mix_callback(mix_udata, pcm, num_decoded_samples);

	// 		if (mixed != num_decoded_samples) {
	// 			samples_offset = mixed;
	// 			audio_buffer_full = true;
	// 		}
	// 	}

	// 	WebMFrame *video_frame;
	// 	if (video_frames_pos >= video_frames_capacity) {

	// 		WebMFrame **video_frames_new = (WebMFrame **)memrealloc(video_frames, ++video_frames_capacity * sizeof(void *));
	// 		ERR_FAIL_COND(!video_frames_new); //Out of memory
	// 		(video_frames = video_frames_new)[video_frames_capacity - 1] = memnew(WebMFrame);
	// 	}
	// 	video_frame = video_frames[video_frames_pos];

	// 	if (!webm->readFrame(video_frame, audio_frame)) //This will invalidate frames
	// 		break; //Can't demux, EOS?

	// 	if (video_frame->isValid())
	// 		++video_frames_pos;
	// };

	bool video_frame_done = false;
	video_frames_pos = 4;
	while (video_frames_pos > 0 && !video_frame_done) {
		if (source->move_frame()) {
			// get the frame data from the source
			if (source->get_frame_data(frame_data)) {
				Ref<Image> img = memnew(Image(854, 480, 0, Image::FORMAT_RGBA8, frame_data));
 				texture->set_data(img); // zero copy send to visual server
				video_frame_done = true;
			}
		}
		video_pos = source->get_frame_time();
		video_frames_pos--;
	}

	if (video_frames_pos == 0 && source->has_ended())
		stop();
}

void VideoStreamPlaybackWmf::set_mix_callback(AudioMixCallback p_callback, void *p_userdata) {}
int VideoStreamPlaybackWmf::get_channels() const { return 1; }
int VideoStreamPlaybackWmf::get_mix_rate() const { return 92; }


////////////////////////
//////Video_Stream//////
////////////////////////

VideoStreamWmf::VideoStreamWmf() {
    file = "";
    audio_track = -1;
}

Ref<VideoStreamPlayback> VideoStreamWmf::instance_playback() {
    Ref<VideoStreamPlaybackWmf> vw = memnew(VideoStreamPlaybackWmf);
    if (vw.is_valid() && vw->open_file(file)) {
	    return vw;
	}
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
