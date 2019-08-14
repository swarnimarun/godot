/*************************************************************************/
/*  video_stream_webm.h                                                  */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef WMF_STREAM_H
#define WMF_STREAM_H

#include "core/io/resource_loader.h"
#include "scene/resources/video_stream.h"

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <stdio.h>
#include <mferror.h>

// Write a wrapper for the SourceReader (Might be only used for creating a simplified implementations of what's needed)
// the implementationals won't be 
class MediaSource {

	/*******************************************************************************/
	/********************* Media Foundation API stuff ******************************/
	/*******************************************************************************/

    IMFSourceReader *reader;

	bool ended;
	bool frame_read;
    IMFSample *sample; // this is basically is a video frame with additional data
    // you also need audio decoding and an audio sample as well for it
    // each sample only holds data of the specific stream

    // output type
    IMFMediaType outputType; // I believe it would be RBG8 or RGBA8 :/

	long long current_frame_count;

    // helper method to get the flags of the media
    HRESULT _get_source_flags(ULONG *pulFlags) {
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
    BOOL _source_can_seek() {
        BOOL bCanSeek = FALSE;
        ULONG flags;
        if (SUCCEEDED(_get_source_flags(reader, &flags))) {
            bCanSeek = ((flags & MFMEDIASOURCE_CAN_SEEK) == MFMEDIASOURCE_CAN_SEEK);
        }
        return bCanSeek;
    }

	// TODO: Expose it
    HRESULT _set_position(const LONGLONG& hnsPosition) {
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
    HRESULT _get_duration(LONGLONG *phnsDuration) {
        PROPVARIANT var;
        HRESULT hr = reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, 
            MF_PD_DURATION, &var);
        if (SUCCEEDED(hr)) {
            hr = PropVariantToInt64(var, phnsDuration);
            PropVariantClear(&var);
        }
        return hr;
    }

public:

	/*******************************************************************************/
	/*********************** Exposed Interface API *********************************/
	/*******************************************************************************/

	enum MediaOutputType {
		OUTPUT_FLAG_RGB8 = 0x0,
		OUTPUT_FLAG_RGBA8 = 0x1,
		OUTPUT_FLAG_RGB16 = 0x2,
		OUTPUT_FLAG_RGBA16 = 0x4,
		TYPE_5 = 0x8,
		TYPE_6 = 0xF
	};

	MediaSource() {
		ended = false;
		reader = NULL;
		hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        // if succeeded in the last operation
        if (SUCCEEDED(hr)) // Initialize the Media Foundation platform.
            hr = MFStartup(MF_VERSION);

		frame_read = false;
		current_frame_count = 0;
	}

    MediaSource(const String &path, short type_flags) {
		reader = NULL;
		ended = false;
        // Initialize the COM library.
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
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
    }

    ~MediaSource() {
		ended = true;
        MFShutdown();
        CoUninitialize();
    }

    bool create_source(const String &path) {
        const wchar_t *path_wchar = path.c_str(); // CharType is wchar_t
		// plan some IMFAttributes that might need to be added
		reader = NULL;
		ended = false;

        if (SUCCEEDED(MFCreateSourceReaderFromURL(path_wchar, NULL, &reader))) {
			frame_read = false;
			current_frame_count = 0;
			return true;
		}
		frame_read = true;
		return false;
	}

	long get_length() {
		if (!reader)
			return 0;
		LONGLONG val;
		_get_duration(&val);
		return val;
	}

	bool get_frame_data(PoolVector<uint8_t> &bytevec) {
		// return the current frame if it's not been read
		if (!frame_read) {
			// woohoo
		}
		frame_read = true;
	}

	bool move_frame(float time, bool forward = true) {
		// move forward by a minimum the provided time
		// get the next best frame after the duration
		// use the loop with the sample duration

		DWORD streamIndex, flags;
		LONGLONG timestamp;

		reader->ReadSample(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0, &streamIndex, &flags, &timestamp, &sample);

        if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
            break;

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

	long long get_frame_pos() const {
		// current frame count
		return current_frame_count;
	}

	float get_frame_time() const {
		LONGLONG time;
		if (SUCCEEDED(sample->GetSampleTime(&time)))
			return time / 1e9; // convert from nanosec to sec
	}

	bool has_ended() const {
		return ended;
	}

	bool set_media_output_type(short type) {
		// change the media output type
		return false;
	}
};


class VideoStreamPlaybackWmf : public VideoStreamPlayback {

	GDCLASS(VideoStreamPlaybackWmf, VideoStreamPlayback);

	String file_name;
	int audio_track; // not going to be decoding audio track for now

    // FileAccess *file;

	// IMFSourceReader *p_reader;
	// IMFByteStream *byte_stream;

	MediaSource *source;

	int video_frames_pos, video_frames_capacity;

	// int num_decoded_samples, samples_offset;
	// AudioMixCallback mix_callback;
	// void *mix_udata;

	bool playing, paused;
	double delay_compensation;
	double time, video_frame_delay, video_pos;

	PoolVector<uint8_t> frame_data;
	Ref<ImageTexture> texture;

	float *pcm; // audio

public:
	VideoStreamPlaybackWmf();
	~VideoStreamPlaybackWmf();

	bool open_file(const String &p_file);

	virtual void stop();
	virtual void play();

	virtual bool is_playing() const;

	virtual void set_paused(bool p_paused);
	virtual bool is_paused() const;

	virtual void set_loop(bool p_enable);
	virtual bool has_loop() const;

	virtual float get_length() const;

	virtual float get_playback_position() const;
	virtual void seek(float p_time);

	virtual void set_audio_track(int p_idx);

	virtual Ref<Texture> get_texture();
	virtual void update(float p_delta);

	virtual void set_mix_callback(AudioMixCallback p_callback, void *p_userdata);
	virtual int get_channels() const;
	virtual int get_mix_rate() const;

private:
	// TODO: decide if we even need this
	inline bool has_enough_video_frames() const { return false; }  // check to see if the video frames are proper
	bool should_process(Ref<Image> &video_frame) { return false; } // this is just to process each from
};

class VideoStreamWmf : public VideoStream {

	GDCLASS(VideoStreamWmf, VideoStream);

	String file;
	int audio_track;

protected:
	static void _bind_methods();

public:
	VideoStreamWmf();

	virtual Ref<VideoStreamPlayback> instance_playback();

	virtual void set_file(const String &p_file);
	String get_file();
	virtual void set_audio_track(int p_track);
};

class ResourceFormatLoaderWmf : public ResourceFormatLoader {
public:
	virtual RES load(const String &p_path, const String &p_original_path = "", Error *r_error = NULL);
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual bool handles_type(const String &p_type) const;
	virtual String get_resource_type(const String &p_path) const;
};

#endif // WMF_STREAM_H
