/* Copyright (C) 2010-2011 Ian MacLarty */
#include <errno.h>
#include <string.h>
#include <list>
#include <map>

#include "ltaudio.h"

#include "ltresource.h"
#include "ltutil.h"

static ALCcontext* audio_context = NULL;
static ALCdevice* audio_device = NULL;

// For one-off playing of a sample.
static std::list<ALuint> temp_sources;

// We keep a record of all LTTrack objects, so
// we can pause them when ltAudioSuspend is called.
// The map values record whether the track is suspended
// (and hence whether it needs to be restarted when
// ltAudioResume is called).
static std::map<LTTrack *, bool> active_tracks;
static bool audio_is_suspended = false;

void ltAudioInit() {
#ifndef LTANDROID
    audio_device = alcOpenDevice(NULL);
    if (audio_device == NULL) {
        ltLog("Unable to open audio device");
        return;
    }
    audio_context = alcCreateContext(audio_device, 0);
    if (audio_context == NULL) {
        ltLog("Unable to create audio context");
        return;
    }
    alcMakeContextCurrent(audio_context);
    alGetError();
#endif
}

void ltAudioTeardown() {
    std::list<ALuint>::iterator it;
    for (it = temp_sources.begin(); it != temp_sources.end(); it++) {
        ALuint src_id = *it;
        alDeleteSources(1, &src_id);
    }
    temp_sources.clear();
    if (audio_context != NULL) {
        alcDestroyContext(audio_context);
        audio_context = NULL;
    }
    if (audio_device != NULL) {
        alcCloseDevice(audio_device);
        audio_device = NULL;
    }
}

LTAudioSample::LTAudioSample(ALuint buf_id, const char *name) : LTObject(LT_TYPE_AUDIOSAMPLE) {
    LTAudioSample::name = new char[strlen(name) + 1];
    strcpy(LTAudioSample::name, name);
    LTAudioSample::buffer_id = buf_id;
}

LTAudioSample::~LTAudioSample() {
    alDeleteBuffers(1, &buffer_id);
    delete[] name;
}

void LTAudioSample::play(LTfloat pitch, LTfloat gain) {
    ALuint source_id;
    alGenSources(1, &source_id);
    alSourcei(source_id, AL_BUFFER, buffer_id);
    alSourcef(source_id, AL_PITCH, pitch);
    alSourcef(source_id, AL_GAIN, gain);
    alSourcei(source_id, AL_LOOPING, AL_FALSE);
    alSourcePlay(source_id);
    temp_sources.push_back(source_id);
}

int LTAudioSample::bytes() {
    ALint n;
    alGetBufferi(buffer_id, AL_SIZE, &n);
    return n;
}

int LTAudioSample::bitsPerDataPoint() {
    ALint n;
    alGetBufferi(buffer_id, AL_BITS, &n);
    return n;
}

int LTAudioSample::channels() {
    ALint n;
    alGetBufferi(buffer_id, AL_CHANNELS, &n);
    return n;
}

int LTAudioSample::numDataPoints() {
    return bytes() / ((bitsPerDataPoint() >> 3) * channels());
}

int LTAudioSample::dataPointsPerSec() {
    ALint n;
    alGetBufferi(buffer_id, AL_FREQUENCY, &n);
    return n;
}

LTdouble LTAudioSample::length() {
    return (LTdouble)numDataPoints() / (LTdouble)dataPointsPerSec();
}

LTTrack::LTTrack() : LTObject(LT_TYPE_TRACK) {
    alGenSources(1, &source_id);
    alSourcef(source_id, AL_PITCH, 1.0f);
    alSourcef(source_id, AL_GAIN, 1.0f);
    alSourcei(source_id, AL_LOOPING, AL_FALSE);
    active_tracks[this] = false;
}

LTTrack::~LTTrack() {
    alDeleteSources(1, &source_id);
    active_tracks.erase(this);
}

void LTTrack::queueSample(LTAudioSample *sample) {
    alSourceQueueBuffers(source_id, 1, &sample->buffer_id);
}

void LTTrack::play() {
    alSourcePlay(source_id);
}

void LTTrack::setLoop(bool loop) {
    alSourcei(source_id, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
}


int LTTrack::numSamples() {
    ALint queued;
    alGetSourcei(source_id, AL_BUFFERS_QUEUED, &queued);
    return queued;
}

int LTTrack::numProcessedSamples() {
    ALint processed;
    alGetSourcei(source_id, AL_BUFFERS_PROCESSED, &processed);
    return processed;
}

int LTTrack::numPendingSamples() {
    return numSamples() - numProcessedSamples();
}

void LTTrack::dequeueProcessedSamples(int n) {
    alSourceUnqueueBuffers(source_id, n, NULL);
}

bool LTTrack::has_field(const char *field_name) {
    return strcmp(field_name, "gain") == 0 || strcmp(field_name, "pitch") == 0;
}

LTfloat LTTrack::get_field(const char *field_name) {
    LTfloat val;
    if (strcmp(field_name, "gain") == 0) {
        alGetSourcef(source_id, AL_GAIN, &val);
        return val;
    }
    if (strcmp(field_name, "pitch") == 0) {
        alGetSourcef(source_id, AL_PITCH, &val);
        return val;
    }
    return 0.0f;
}

void LTTrack::set_field(const char *field_name, LTfloat value) {
    if (strcmp(field_name, "gain") == 0) {
        alSourcef(source_id, AL_GAIN, value);
        return;
    }
    if (strcmp(field_name, "pitch") == 0) {
        alSourcef(source_id, AL_PITCH, value);
        return;
    }
}

void ltAudioSuspend() {
    if (!audio_is_suspended) {
        {
            std::map<LTTrack *, bool>::iterator it;
            for (it = active_tracks.begin(); it != active_tracks.end(); it++) {
                ALint state;
                alGetSourcei(it->first->source_id, AL_SOURCE_STATE, &state);
                if (state == AL_PLAYING) {
                    alSourcePause(it->first->source_id);
                    active_tracks[it->first] = true;
                }
            }
        }
        {
            std::list<ALuint>::iterator it;
            for (it = temp_sources.begin(); it != temp_sources.end(); it++) {
                alSourcePause(*it);
            }
        }
        audio_is_suspended = true;
    }
}

void ltAudioResume() {
    if (audio_is_suspended) {
        {
            std::map<LTTrack *, bool>::iterator it;
            for (it = active_tracks.begin(); it != active_tracks.end(); it++) {
                if (it->second) { // If it is suspended.
                    alSourcePlay(it->first->source_id);
                    active_tracks[it->first] = false;
                }
            }
        }
        {
            std::list<ALuint>::iterator it;
            for (it = temp_sources.begin(); it != temp_sources.end(); it++) {
                alSourcePlay(*it);
            }
        }
        audio_is_suspended = false;
    }
}

static bool delete_source_if_finished(ALuint source_id) {
    ALint state;
    alGetSourcei(source_id, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING) {
        return false;
    } else {
        alDeleteSources(1, &source_id);
        return true;
    }
}

void ltAudioGC() {
    if (!audio_is_suspended) {
        // Only collect non-playing temp tracks if audio not suspended,
        // because they will all be paused (and hence non-playing) if
        // audio is suspended, so they will all get collected.
        temp_sources.remove_if(delete_source_if_finished);
    }
}

static int read_4_byte_little_endian_int(LTResource *rsc) {
    int val = 0;
    ltReadResource(rsc, &val, 4);
    return val;
}

static int read_2_byte_little_endian_int(LTResource *rsc) {
    int val = 0;
    ltReadResource(rsc, &val, 2);
    return val;
}

LTAudioSample *ltReadAudioSample(const char *path, const char *name) {
    char chunkid[5];
    memset(chunkid, 0, 5);

    LTResource *rsc = ltOpenResource(path);
    if (rsc == NULL) {
        ltLog("Unable to open resource %s", path);
        return NULL;
    }

    ltReadResource(rsc, chunkid, 4);
    if (strcmp(chunkid, "RIFF") != 0) {
        ltLog("RIFF chunk id not found in %s", path);
        ltCloseResource(rsc);
        return NULL;
    }

    read_4_byte_little_endian_int(rsc); // file size

    ltReadResource(rsc, chunkid, 4);

    if (strcmp(chunkid, "WAVE") != 0) {
        ltLog("WAVE format id not found in %s", path);
        ltCloseResource(rsc);
        return NULL;
    }

    ltReadResource(rsc, chunkid, 4);
    if (strcmp(chunkid, "fmt ") != 0) {
        ltLog("fmt chunk id not found in %s", path);
        ltCloseResource(rsc);
        return NULL;
    }

    int chunksize = read_4_byte_little_endian_int(rsc);
    if (chunksize != 16) {
        ltLog("fmt chunk size is not 16");
        ltCloseResource(rsc);
        return NULL;
    }

    int format_id = read_2_byte_little_endian_int(rsc);
    if (format_id != 1) {
        ltLog("Format id should be 1 for PCM");
        ltCloseResource(rsc);
        return NULL;
    }

    int num_channels = read_2_byte_little_endian_int(rsc);
    if (num_channels != 1 && num_channels != 2) {
        ltLog("Unsupported number of channels: %d in %s", num_channels, path);
        ltCloseResource(rsc);
        return NULL;
    }

    int sample_rate = read_4_byte_little_endian_int(rsc);

    read_4_byte_little_endian_int(rsc); // byte rate
    read_2_byte_little_endian_int(rsc); // bytes per sample (all channels)

    int bits_per_sample = read_2_byte_little_endian_int(rsc);
    if (bits_per_sample != 8 && bits_per_sample != 16) {
        ltLog("Unsupported bits per sample %d in %s", bits_per_sample, path);
        ltCloseResource(rsc);
        return NULL;
    }

    int bytes_per_sample = bits_per_sample / 8;

    ltReadResource(rsc, chunkid, 4);
    if (strcmp(chunkid, "data") != 0) {
        ltLog("Data chunk not found in %s", path);
        ltCloseResource(rsc);
        return NULL;
    }

    int data_size = read_4_byte_little_endian_int(rsc);

    unsigned char *data = new unsigned char[data_size];

    int num_bytes_read = ltReadResource(rsc, data, data_size);

    if (num_bytes_read != data_size) {
        ltLog("Unable to read all bytes in %s: %s", path, strerror(errno));
        ltCloseResource(rsc);
        delete[] data;
        return NULL;
    }

    unsigned char trailing;
    num_bytes_read = ltReadResource(rsc, &trailing, 1);
    if (num_bytes_read != 0) {
        ltLog("Extra bytes at end of %s", path);
        ltCloseResource(rsc);
        delete[] data;
        return NULL;
    }

    ltCloseResource(rsc);

    ALuint buf_id;
    ALenum format;
    alGenBuffers(1, &buf_id);
    if (num_channels == 1 && bytes_per_sample == 2) {
        format = AL_FORMAT_MONO16;
    } else if (num_channels == 2 && bytes_per_sample == 2) {
        format = AL_FORMAT_STEREO16;
    } else if (num_channels == 1 && bytes_per_sample == 1) {
        format = AL_FORMAT_MONO8;
    } else {
        format = AL_FORMAT_STEREO8;
    }
    alBufferData(buf_id, format, data, data_size, sample_rate);
    delete[] data;

    ALenum err = alGetError();
    if (err != AL_NO_ERROR) {
        ltLog("alBufferData returned error %x", err);
        return NULL;
    }
    
    LTAudioSample *buf = new LTAudioSample(buf_id, name);
    return buf;
}
