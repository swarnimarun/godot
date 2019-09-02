#include "wmf_stream.h"

#include "core/os/file_access.h" // just to test permissions
#include "core/os/os.h" // unused at the moment
#include "core/project_settings.h" // to fetch information

#include "propvarutil.h" // wmf stuff

// WMF Ptr with convenience features
template <class T>
class WMFPtr {
    T *p; // internally held pointer 
public:
    WMFPtr(T *ptr) {
        p = ptr;
    }
    WMFPtr(const WMFptr<T> &pt) {
        p = pt.p;
    }
    void release() { p->Release(); }
    T *operator->() const {
        ERR_FAIL_COND_V(p == NULL, NULL);
        return p.operator->();
    }
    operator bool() {
        return p.operator!=(NULL);
    }

    T *ptr() const { return p; }
    // ? QueryInterface cast operation ?
};
// use WMFPtrs in the delete pointers function

class MediaType {
    IMFMediaType *type;
    // hold lots of direct values
    UINT32 width, height; // resolution
public:
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
    int compare(const MediaType &t) {
        if (type == NULL)
            return -1;
        // check which one of the two is larger
        int w = width == 0 ? 0 : width > t->width ? width - t->width : t->width - width;
        int h = height == 0 ? 0 : height > t->height ? height - t->height : t->height - height;
        return w + h;
    }
};

class WMFVideo {
    WMFPtr<IMFSourceReader> reader;
    WMFPtr<IMFSample> frame;

    UINT32 width, height;
    DWORD streamIndex;

    int current_frame;
    bool ended;
private:
    void delete_pointers() {
        if (reader)
            reader.release();
        if (frame)
            frame.release();
    }
    DWORD find_best_stream() {
        // find best stream
        DWORD s_idx = 0, cnt = 0;
        IMFMediaType *n_type;
        GUID n_guid;
        MediaType best_media;
        while (true) {
            while(true) {
                if (SUCCEEDED(hr = reader->GetNativeMediaType(s_idx, cnt, &n_type))) {
                    if (SUCCEEDED(hr = n_type->GetGUID(MF_MT_MAJOR_TYPE, &n_guid))) {
                        if (n_guid == MFMediaType_Video) {
                            MediaType mt(n_type);
                            if (mt.compare(best_media) > 0) {
                                best_media.release();
                                best_media = mt;
                            } else {
                                mt.release();
                            }
                        }
                    } else {
                        break;
                    }
                } else {
                    break;
                }
                cnt += 1;
            }
            s_idx += 1;
        }
        return best_media.stream;
    }
public:
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

    void create_source(const String &path) {
        // create the source from the media URL
	    const wchar_t *path_wchar = path.c_str(); // CharType is wchar_t

        HRESULT hr = MFCreateSourceReaderFromURL(path.c_str(), NULL, &reader);
        if (SUCCEEDED(hr)) {
            current_frame = 0;
            streamIndex = find_best_stream();
            IMFMediaType *outputType;
            hr = MFCreateMediaType(&outputType);
            if (SUCCEEDED(hr)) {
                hr = outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
                if (SUCCEEDED(hr)) {
                    hr = outputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB8);
                    reader->SetCurrentMediaType(streamIndex, NULL, outputType);
                    ended = false;
                    return true;
                }
                outputType->Release();
            }
        }
        return false;
    }
    void move_frame() {
        // move forward by a minimum the provided time
        // get the next best frame after the duration
        // use the loop with the sample duration

        DWORD flags = 0;
        LONGLONG timestamp;

        // make sure the actual video stream is being read.
        HRESULT hr = reader->ReadSample(
                streamIndex,
                0, 0, &flags, &timestamp, &frame);

        if (FAILED(hr) || !frame) {
            return false;
        }

        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
            ended = true;
        }

        current_frame++;
        return true;
    }

    bool get_frame_data(PoolVector<uint8_t> &bytevec) {
        // return the current frame if it's not been read
        PoolVector<uint8_t>::Write wrt = bytevec.write();
        uint8_t *w = wrt.ptr();
        // create a IMFMediaBuffer using the ConvertToContigousBuffer
        IMFMediaBuffer *media_buf;
        HRESULT hr = frame->ConvertToContiguousBuffer(&media_buf);
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
                if (CURR_SIZE == (width * height * 3)) {
                    for (int j = 0; j < height; j++) {
                        for (int i = 0; i < width; i++) {
                            *w++ = *(buffer + i + j * width);
                            *w++ = *(buffer + 1 + i + j * width);
                            *w++ = *(buffer + 2 + i + j * width);
                            *w++ = 255;
                        }
                    }
                }
                media_buf->Unlock();
                return true;
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


    int get_frame_pos() const {
        // current frame count
        return current_frame;
    }
    float get_frame_time() const {
        LONGLONG time;
        if (!frame)
            return 0;
        if (SUCCEEDED(sample->GetSampleTime(&time)))
            return time / 1e9; // convert from nanosec to sec
        return 0;
    }
};

// copy code from original file
