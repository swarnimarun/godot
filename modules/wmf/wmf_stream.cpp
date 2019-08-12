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

// Write a wrapper for the SourceReader (Might be only used for creating a simplified implementations of what's needed)
// the implementationals won't be 
class MediaSource {
    IMFSourceReader *reader;
    IMFSample *sample; // this is basically is a video frame with additional data
    // you also need audio decoding and an audio sample as well for it
    // each sample only holds data of the specific stream

    // output type
    IMFMediaType outputType; // I believe it would be RBG8 or RGBA8 :/

    // helper method to get the flags of the media
    HRESULT GetSourceFlags(ULONG *pulFlags) {
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

public:
    MediaSource() {
        // ! TODO: Finish this function either use flag variables or just make it a parameterized constructor
        // Initialize the COM library.
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        // if succeeded in the last operation
        if (SUCCEEDED(hr)) {
            // Initialize the Media Foundation platform.
            hr = MFStartup(MF_VERSION);
            if (SUCCEEDED(hr)) {
                // Create the source reader to read the input file.
                // ? hr = CreateSource(path);
            }
        }
    }
    ~MediaSource() {
        MFShutdown();
        CoUninitialize();
    }

    HRESULT CreateSource(const wchar_t *path) {
        // plan some IMFAttributes that might need to be added
        return MFCreateSourceReaderFromURL(path, NULL, &reader);
    }

    // check to see if the media is seekable
    BOOL SourceCanSeek() {
        BOOL bCanSeek = FALSE;
        ULONG flags;
        if (SUCCEEDED(GetSourceFlags(reader, &flags))) {
            bCanSeek = ((flags & MFMEDIASOURCE_CAN_SEEK) == MFMEDIASOURCE_CAN_SEEK);
        }
        return bCanSeek;
    }

    HRESULT SetPosition(const LONGLONG& hnsPosition) {
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
    HRESULT GetDuration(LONGLONG *phnsDuration) {
        PROPVARIANT var;
        HRESULT hr = reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, 
            MF_PD_DURATION, &var);
        if (SUCCEEDED(hr)) {
            hr = PropVariantToInt64(var, phnsDuration);
            PropVariantClear(&var);
        }
        return hr;
    }
};

////////////////////////
/////Video_PLayback/////
////////////////////////

VideoStreamPlaybackWmf::VideoStreamPlaybackWmf() {
    HRESULT hr = S_OK;

    p_reader = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;

            if (FAILED(hr)) {
                ERR_PRINT("Couldn't read from the source file properly.");
            }
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

		// delete_pointers();

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
    if ((!playing || paused) || !video)
		return;

	time += p_delta;

	if (time < video_pos) {
		return;
	}

	bool audio_buffer_full = false;

	if (samples_offset > -1) {

		//Mix remaining samples
		const int to_read = num_decoded_samples - samples_offset;
		const int mixed = mix_callback(mix_udata, pcm + samples_offset * webm->getChannels(), to_read);
		if (mixed != to_read) {

			samples_offset += mixed;
			audio_buffer_full = true;
		} else {

			samples_offset = -1;
		}
	}

	const bool hasAudio = (audio && mix_callback);
	while ((hasAudio && !audio_buffer_full && !has_enough_video_frames()) ||
			(!hasAudio && video_frames_pos == 0)) {

		if (hasAudio && !audio_buffer_full && audio_frame->isValid() &&
				audio->getPCMF(*audio_frame, pcm, num_decoded_samples) && num_decoded_samples > 0) {

			const int mixed = mix_callback(mix_udata, pcm, num_decoded_samples);

			if (mixed != num_decoded_samples) {
				samples_offset = mixed;
				audio_buffer_full = true;
			}
		}

		WebMFrame *video_frame;
		if (video_frames_pos >= video_frames_capacity) {

			WebMFrame **video_frames_new = (WebMFrame **)memrealloc(video_frames, ++video_frames_capacity * sizeof(void *));
			ERR_FAIL_COND(!video_frames_new); //Out of memory
			(video_frames = video_frames_new)[video_frames_capacity - 1] = memnew(WebMFrame);
		}
		video_frame = video_frames[video_frames_pos];

		if (!webm->readFrame(video_frame, audio_frame)) //This will invalidate frames
			break; //Can't demux, EOS?

		if (video_frame->isValid())
			++video_frames_pos;
	};

	bool video_frame_done = false;
	while (video_frames_pos > 0 && !video_frame_done) {

		WebMFrame *video_frame = video_frames[0];

		// It seems VPXDecoder::decode has to be executed even though we might skip this frame
		if (video->decode(*video_frame)) {

			VPXDecoder::IMAGE_ERROR err;
			VPXDecoder::Image image;

			if (should_process(*video_frame)) {

				if ((err = video->getImage(image)) != VPXDecoder::NO_FRAME) {

					if (err == VPXDecoder::NO_ERROR && image.w == webm->getWidth() && image.h == webm->getHeight()) {

						PoolVector<uint8_t>::Write w = frame_data.write();
						bool converted = false;

						if (image.chromaShiftW == 0 && image.chromaShiftH == 0 && image.cs == VPX_CS_SRGB) {

							uint8_t *wp = w.ptr();
							unsigned char *rRow = image.planes[2];
							unsigned char *gRow = image.planes[0];
							unsigned char *bRow = image.planes[1];
							for (int i = 0; i < image.h; i++) {
								for (int j = 0; j < image.w; j++) {
									*wp++ = rRow[j];
									*wp++ = gRow[j];
									*wp++ = bRow[j];
									*wp++ = 255;
								}
								rRow += image.linesize[2];
								gRow += image.linesize[0];
								bRow += image.linesize[1];
							}
							converted = true;
						} else if (image.chromaShiftW == 1 && image.chromaShiftH == 1) {

							yuv420_2_rgb8888(w.ptr(), image.planes[0], image.planes[1], image.planes[2], image.w, image.h, image.linesize[0], image.linesize[1], image.w << 2);
							// 								libyuv::I420ToARGB(image.planes[0], image.linesize[0], image.planes[2], image.linesize[2], image.planes[1], image.linesize[1], w.ptr(), image.w << 2, image.w, image.h);
							converted = true;
						} else if (image.chromaShiftW == 1 && image.chromaShiftH == 0) {

							yuv422_2_rgb8888(w.ptr(), image.planes[0], image.planes[1], image.planes[2], image.w, image.h, image.linesize[0], image.linesize[1], image.w << 2);
							// 								libyuv::I422ToARGB(image.planes[0], image.linesize[0], image.planes[2], image.linesize[2], image.planes[1], image.linesize[1], w.ptr(), image.w << 2, image.w, image.h);
							converted = true;
						} else if (image.chromaShiftW == 0 && image.chromaShiftH == 0) {

							yuv444_2_rgb8888(w.ptr(), image.planes[0], image.planes[1], image.planes[2], image.w, image.h, image.linesize[0], image.linesize[1], image.w << 2);
							// 								libyuv::I444ToARGB(image.planes[0], image.linesize[0], image.planes[2], image.linesize[2], image.planes[1], image.linesize[1], w.ptr(), image.w << 2, image.w, image.h);
							converted = true;
						}

						if (converted) {
							Ref<Image> img = memnew(Image(image.w, image.h, 0, Image::FORMAT_RGBA8, frame_data));
							texture->set_data(img); //Zero copy send to visual server
							video_frame_done = true;
						}
					}
				}
			}
		}

		video_pos = video_frame->time;
		memmove(video_frames, video_frames + 1, (--video_frames_pos) * sizeof(void *));
		video_frames[video_frames_pos] = video_frame;
	}

	if (video_frames_pos == 0 && webm->isEOS())
		stop();
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
