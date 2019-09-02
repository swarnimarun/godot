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


// WMF Ptr with convenience features
template <class T>
class WMFPtr {
    T *p; // internally held pointer 
public:
	WMFPtr() {}
    WMFPtr(T *ptr) {
        p = ptr;
    }
    WMFPtr(const WMFPtr<T> &pt) {
        p = pt.p;
    }
    void release() { p->Release(); }
    bool is_valid() const  {
        return p != NULL;
    }
    T *ptr() const { return p; }
    // ? QueryInterface cast operation ?
};
// use WMFPtrs in the delete pointers function

class MediaType {
    IMFMediaType *type;
    // hold lots of direct values
public:
    UINT32 width, height; // resolution
    DWORD stream;
    // for convenince media type ops
    MediaType() {
        width = 0;
        height = 0;
        stream = 0;
    }
    MediaType(IMFMediaType *p_type, DWORD p_stream) {
        type = p_type;
        width = 0;
        height = 0;
        stream = p_stream;
        // setup width and height
        MFGetAttributeSize(type, MF_MT_FRAME_SIZE, &width, &height);
        // print the values you want to
        // printf("Size: (%u, %u) \n", width, height);
    }
    void release() {
        ERR_FAIL_COND(!type);
        type->Release();
    }
    int compare(int t_width, int t_height) {
        // check which one of the two is larger
        int w = t_width == 0 ? 0 : width > t_width ? width - t_width : t_width - width;
        int h = t_height == 0 ? 0 : height > t_height ? height - t_height : t_height - height;
        return w + h;
    }
};

class WMFVideo {
    IMFSourceReader *reader;
    IMFSample *frame;

    UINT32 width, height;
    DWORD streamIndex;

    int current_frame;
    bool ended;
private:
    void delete_pointers() {
        if (reader)
            reader->Release();
        if (frame)
            frame->Release();
    }

    bool configure() {
        width = 854;
		height = 480;
        DWORD s_idx = 0, cnt = 0;
        IMFMediaType *n_type;
        GUID major_type, subtype;
        MediaType best_media;
		HRESULT hr;
        while (true) {
			hr = reader->GetNativeMediaType(s_idx, cnt, &n_type);
			if (hr == MF_E_NO_MORE_TYPES) {
				s_idx++;
				cnt = 0;
				hr = S_OK;
				continue;
			}
			if (SUCCEEDED(hr)) {
				if (SUCCEEDED(hr = n_type->GetGUID(MF_MT_MAJOR_TYPE, &major_type)) && SUCCEEDED(hr = n_type->GetGUID(MF_MT_SUBTYPE, &subtype))) {
					if (major_type == MFMediaType_Video) {
			     	    print_line("selecting best stream");
						// find the closest media to the width and height required
						MediaType media(n_type, s_idx);
						if (best_media.stream < 0 ||
						media.compare(width, height) < best_media.compare(width, height) ||
						(media.compare(width, height) == best_media.compare(width, height) && media.width > best_media.width) ||
						(media.compare(width, height) == best_media.compare(width, height) && media.width == best_media.width && media.height > best_media.height)) {
							best_media = media;
							best_media.stream = s_idx;
						}
					}
				}
			} else {
				break;
			}
			cnt += 1;
        }
		IMFMediaType *media_type;
		if (SUCCEEDED(MFCreateMediaType(&media_type)) &&
			SUCCEEDED(media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video)) &&
			SUCCEEDED(media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB8)) &&
			SUCCEEDED(media_type->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive)) &&
			SUCCEEDED(MFSetAttributeSize(media_type, MF_MT_FRAME_SIZE, best_media.width, best_media.height)) &&
			SUCCEEDED(media_type->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, 1)) &&
			SUCCEEDED(media_type->SetUINT32(MF_MT_SAMPLE_SIZE, best_media.height * best_media.width * 3)) &&
			SUCCEEDED(media_type->SetUINT32(MF_MT_DEFAULT_STRIDE, best_media.width * 3))
			) {
			// created media type with values setup
			if (
				SUCCEEDED(reader->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, false)) &&
				SUCCEEDED(reader->SetStreamSelection((DWORD)best_media.stream, true)) &&
				SUCCEEDED(reader->SetCurrentMediaType((DWORD)best_media.stream, NULL, media_type))) {
				// setup all the extra variables
				streamIndex = best_media.stream;
				print_line("successfully setup stream");
			}
			streamIndex = best_media.stream;
		} else { return false; }
		return true;
    }
public:
	uint32_t get_width() const { return width; }
	uint32_t get_height() const { return height; }

	bool has_ended() const { return ended; }

    WMFVideo() {
        // initialize the startup related stuff
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        if (SUCCEEDED(hr)) 
            hr = MFStartup(MF_VERSION);

        current_frame = 0;
        ended = true;
    }
    ~WMFVideo() {
        delete_pointers();
        MFShutdown();
        CoUninitialize();
    }

    bool create_source(const String &path) {
        // create the source from the media URL
	    const wchar_t *path_wchar = path.c_str(); // CharType is wchar_t
	   HRESULT hr = MFCreateSourceReaderFromURL(path.c_str(), NULL, &reader);
        if (SUCCEEDED(hr)) {
            current_frame = 0;
			if (configure())
			    return true;
        }
        return false;
    }

    bool move_frame() {
        // move forward by a minimum the provided time
        // get the next best frame after the duration
        // use the loop with the sample duration

        DWORD flags = 0;
        LONGLONG timestamp;
        // make sure the actual video stream is being read.
        HRESULT hr = reader->ReadSample(
                streamIndex,
                0, 0, &flags, &timestamp, &frame);
        if (FAILED(hr)) {
            return false;
        }

        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
            ended = true;
        }

        current_frame++;
        return true;
    }

    bool get_frame_data(PoolVector<uint8_t> &bytevec) {
    	print_line("frame data function");
		{
			// return the current frame if it's not been read
			PoolVector<uint8_t>::Write wrt = bytevec.write();
			uint8_t *w = wrt.ptr();
			// create a IMFMediaBuffer using the ConvertToContigousBuffer
			IMFMediaBuffer *media_buf;
			HRESULT hr = frame->ConvertToContiguousBuffer(&media_buf);
			if (SUCCEEDED(hr)) {
				// convert to a IMF2DBuffer using the QueryInterface function on MediaBuffer
				print_line("get the contigous buffer");
				BYTE *buffer;
				DWORD CURR_SIZE;
				IMF2DBuffer *d_buff;
				hr = media_buf->QueryInterface<IMF2DBuffer>(&d_buff);
				if (SUCCEEDED(hr)) {
				    print_line("lock 2d interface");
					// use the Lock2D if the isContigous is true
					// otherwise use Lock which is slower but guarantees contigous memory
					LONG pitch;
					hr = d_buff->Lock2D(&buffer, &pitch);
					for (int j = 0; j < 480; j++) {
						for (int i = 0; i < 854; i++) {
							*w++ = *(buffer + i + pitch * j);
							*w++ = *(buffer + 1 + i + pitch * j);
							*w++ = *(buffer + 2 + i + pitch * j);
							*w++ = 255;
						}
					}
					hr = d_buff->Unlock2D();
					// frame_read = true;
					return true;
				} else if (SUCCEEDED(media_buf->Lock(&buffer, NULL, &CURR_SIZE))) {
				    print_line("get the frame data " + String::num_uint64(CURR_SIZE));
					if (CURR_SIZE >= (width * height * 3)) {
						for (int j = 0; j < height; j++) {
							for (int i = 0; i < width * 3; i = i + 3) {
								*w++ = *(buffer + i + j * width * 3);
								*w++ = *(buffer + 1 + i + j * width * 3);
								*w++ = *(buffer + 2 + i + j * width * 3);
								*w++ = 255;
							}
						}
					    media_buf->Unlock();
					    return true;
					}
						for (int j = 0; j < height; j++) {
							for (int i = 0; i < width * 3; i = i + 3) {
								*w++ = *(buffer + (j > CURR_SIZE ? CURR_SIZE : j) );
								*w++ = *(buffer + (j > CURR_SIZE ? CURR_SIZE : j) );
								*w++ = *(buffer + (j > CURR_SIZE ? CURR_SIZE : j) );
								*w++ = 255;
							}
						}					
					media_buf->Unlock();
				    return true;
				}
			}
		}
		{
			print_line("clear the screen with magenta like color");
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
		}
        return true;
    }


    int get_frame_pos() const {
        // current frame count
        return current_frame;
    }
    float get_frame_time() const {
        LONGLONG time;
        if (!frame)
            return 0;
        if (SUCCEEDED(frame->GetSampleTime(&time)))
            return time / 1e9; // convert from nanosec to sec
        return 0;
    }
};

class VideoStreamPlaybackWmf : public VideoStreamPlayback {

	GDCLASS(VideoStreamPlaybackWmf, VideoStreamPlayback);

	String file_name;
	int audio_track; // not going to be decoding audio track for now

    // FileAccess *file;

	// IMFSourceReader *p_reader;
	// IMFByteStream *byte_stream;

	WMFVideo *source;

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
