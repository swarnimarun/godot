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

class VideoStreamPlaybackWmf : public VideoStreamPlayback {

	GDCLASS(VideoStreamPlaybackWmf, VideoStreamPlayback);

	String file_name;
	int audio_track; // not going to be decoding audio track for now

    // FileAccess *file;

	IMFSourceReader *p_reader;
	IMFByteStream *byte_stream;

	// WebMDemuxer *webm;
	// VPXDecoder *video;
	// OpusVorbisDecoder *audio;

	// WebMFrame **video_frames, *audio_frame;
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

	void delete_pointers() {} // TODO: put some actual code \\ this is to clean stuff up
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
