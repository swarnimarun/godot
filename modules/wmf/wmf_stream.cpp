#include "wmf_stream.h"

#include "core/os/file_access.h"
#include "core/os/os.h"
#include "core/project_settings.h"

VideoStreamPlaybackWmf::VideoStreamPlayback() {
   
}

bool VideoStreamPlaybackWmf::open_file(const String &p_file) {
    String st = ProjectSettings::get_singleton()->globalize_path(p_file);
    print_line(st);
}

void VideoStreamPlaybackWmf::stop() {
    paused = true;
}
void VideoStreamPlaybackWmf::play() {
    playing = true;
}

bool VideoStreamPlaybackWmf::is_playing() const { return playing; }

void VideoStreamPlaybackWmf::set_paused(bool p_paused) { paused = p_paused; }

bool VideoStreamPlaybackWmf::is_paused() const { return paused; }

void VideoStreamPlaybackWmf::set_loop(bool p_enable) {}

bool VideoStreamPlaybackWmf::has_loop() const { return false; }

float VideoStreamPlaybackWmf::get_length() const { return 0.0; }
float VideoStreamPlaybackWmf::get_playback_position() const {  }

void VideoStreamPlaybackWmf::seek(float p_time) {}
void VideoStreamPlaybackWmf::set_audio_track(int p_idx) {}

Ref<Texture> VideoStreamPlaybackWmf::get_texture() { return Ref<Texture>(); }

void VideoStreamPlaybackWmf::update(float p_delta) {}

void VideoStreamPlaybackWmf::set_mix_callback(AudioMixCallback p_callback, void *p_userdata) {}
int VideoStreamPlaybackWmf::get_channels() const { return 3; }
int VideoStreamPlaybackWmf::get_mix_rate() const { return 0; }

