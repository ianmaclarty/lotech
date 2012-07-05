#include "lt.h"

LT_INIT_IMPL(ltaudio)

static const char *oal_errstr(ALenum err) {
    switch (err) {
        case AL_NO_ERROR: return "AL_NO_ERROR";
        case AL_INVALID_NAME: return "AL_INVALID_NAME";
        case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
        case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
        case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
        case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";
        default: return "unknown";
    }
}

#define check_for_errors \
    {   \
        ALenum err = alGetError(); \
        if (err != AL_NO_ERROR) { \
            ltLog("%s:%d: OpenAL error: %s", __FILE__, __LINE__, oal_errstr(err)); \
        } \
    }

static ALCcontext* audio_context = NULL;
static ALCdevice* audio_device = NULL;

struct Source {
    ALuint source_id;
    bool free;
    bool temp;
    bool play_on_resume;
    Source() {
        alGenSources(1, &source_id);
        check_for_errors
        free = true;
        temp = false;
        play_on_resume = false;
    }
};

static std::vector<Source> sources;

static ALuint get_source(bool temp);
static void free_source(ALuint source_id);
static void fixup_source_state(ALuint source_id);

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
    check_for_errors
#endif
}

void ltAudioTeardown() {
#ifndef LTANDROID
    for (unsigned i = 0; i < sources.size(); i++) {
        Source s = sources[i];
        alSourcei(s.source_id, AL_BUFFER, 0);
        alDeleteSources(1, &s.source_id);
    }
    sources.clear();
    if (audio_context != NULL) {
        alcDestroyContext(audio_context);
        audio_context = NULL;
    }
    if (audio_device != NULL) {
        alcCloseDevice(audio_device);
        audio_device = NULL;
    }
#endif
}

LTAudioSample::LTAudioSample(ALuint buf_id, const char *name) {
    LTAudioSample::name = new char[strlen(name) + 1];
    strcpy(LTAudioSample::name, name);
    LTAudioSample::buffer_id = buf_id;
}

LTAudioSample::~LTAudioSample() {
    // XXX What happens to the sources the buffer is attached to?
    alDeleteBuffers(1, &buffer_id);
    check_for_errors
    delete[] name;
}

void LTAudioSample::play(LTfloat pitch, LTfloat gain) {
    ALuint source_id = get_source(true);
    alSourceQueueBuffers(source_id, 1, &buffer_id);
    alSourcef(source_id, AL_PITCH, pitch);
    alSourcef(source_id, AL_GAIN, gain);
    alSourcei(source_id, AL_LOOPING, AL_FALSE);
    alSourcePlay(source_id);
    check_for_errors
}

int LTAudioSample::bytes() {
    ALint n;
    alGetBufferi(buffer_id, AL_SIZE, &n);
    check_for_errors
    return n;
}

int LTAudioSample::bitsPerDataPoint() {
    ALint n;
    alGetBufferi(buffer_id, AL_BITS, &n);
    check_for_errors
    return n;
}

int LTAudioSample::channels() {
    ALint n;
    alGetBufferi(buffer_id, AL_CHANNELS, &n);
    check_for_errors
    return n;
}

int LTAudioSample::numDataPoints() {
    return bytes() / ((bitsPerDataPoint() >> 3) * channels());
}

int LTAudioSample::dataPointsPerSec() {
    ALint n;
    alGetBufferi(buffer_id, AL_FREQUENCY, &n);
    check_for_errors
    return n;
}

LTdouble LTAudioSample::length() {
    return (LTdouble)numDataPoints() / (LTdouble)dataPointsPerSec();
}

LT_REGISTER_TYPE(LTAudioSample, "lt.Sample", "lt.Object")

LTTrack::LTTrack() {
    source_id = get_source(false);
    alSourcef(source_id, AL_PITCH, 1.0f);
    alSourcef(source_id, AL_GAIN, 1.0f);
    alSourcei(source_id, AL_LOOPING, AL_FALSE);
    was_playing = false;
    check_for_errors
}

LTTrack::~LTTrack() {
    free_source(source_id);
}

static ALuint get_source(bool temp) {
    for (unsigned i = 0; i < sources.size(); i++) {
        Source s = sources[i];
        if (s.free) {
            s.temp = temp;
            s.free = false;
            return s.source_id;
        }
    }
    Source new_source;
    new_source.temp = temp;
    new_source.free = false;
    sources.push_back(new_source);
    return new_source.source_id;
}

static void free_source(ALuint source_id) {
    for (unsigned i = 0; i < sources.size(); i++) {
        Source s = sources[i];
        if (s.source_id == source_id) {
            assert(!s.free);
            alSourcei(source_id, AL_BUFFER, 0);
            alSourceStop(source_id);
            check_for_errors
            s.free = true;
            return;
        }
    }
    assert(false); // source_id not in sources.
}

static void fixup_source_state(ALuint source_id) {
    // There seems to be a bug in the Mac OS X OpenAL implementation
    // where the state is not set to AL_STOPPED after all samples have finished.
    ALint state;
    ALint looping;
    ALint queued;
    ALint processed;
    alGetSourcei(source_id, AL_SOURCE_STATE, &state);
    alGetSourcei(source_id, AL_LOOPING, &looping);
    check_for_errors
    if (state == AL_PLAYING && !looping) {
        alGetSourcei(source_id, AL_BUFFERS_QUEUED, &queued);
        alGetSourcei(source_id, AL_BUFFERS_PROCESSED, &processed);
        check_for_errors
        if (queued == processed) {
            alSourceStop(source_id);
            check_for_errors
        }
    }
}

void LTTrack::on_activate() {
    if (was_playing) {
        alSourcePlay(source_id);
        check_for_errors
        was_playing = false;
    }
}

void LTTrack::on_deactivate() {
    fixup_source_state(source_id);
    ALint state;
    alGetSourcei(source_id, AL_SOURCE_STATE, &state);
    check_for_errors
    if (state == AL_PLAYING) {
        was_playing = true;
    }
    alSourcePause(source_id);
    check_for_errors
}

void LTTrack::queueSample(LTAudioSample *sample, int ref) {
    alSourceQueueBuffers(source_id, 1, &sample->buffer_id);
    check_for_errors
    queued_samples.push_front(std::pair<LTAudioSample*, int>(sample, ref));
}

void LTTrack::play() {
    if (active) {
        ALint state;
        alGetSourcei(source_id, AL_SOURCE_STATE, &state);
        check_for_errors
        if (state != AL_PLAYING) {
            alSourcePlay(source_id);
        }
        check_for_errors
        was_playing = false;
    } else {
        was_playing = true;
    }
}

void LTTrack::pause() {
    alSourcePause(source_id);
    check_for_errors
    was_playing = false;
}

void LTTrack::stop() {
    alSourceStop(source_id);
    check_for_errors
    was_playing = false;
}

void LTTrack::rewind() {
    alSourceRewind(source_id);
    check_for_errors
    was_playing = false;
}

void LTTrack::setLoop(bool loop) {
    alSourcei(source_id, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    check_for_errors
}


int LTTrack::numSamples() {
    ALint queued;
    alGetSourcei(source_id, AL_BUFFERS_QUEUED, &queued);
    check_for_errors
    return queued;
}

int LTTrack::numProcessedSamples() {
    ALint processed;
    alGetSourcei(source_id, AL_BUFFERS_PROCESSED, &processed);
    check_for_errors
    return processed;
}

int LTTrack::numPendingSamples() {
    return numSamples() - numProcessedSamples();
}

void LTTrack::dequeueSamples(lua_State *L, int track_index, int n) {
    alSourceUnqueueBuffers(source_id, n, NULL);
    check_for_errors
    std::list<std::pair<LTAudioSample*, int> >::iterator it = queued_samples.begin();
    for (int i = 0; (i < n && !queued_samples.empty()); i++) {
        ltLuaDelRef(L, track_index, queued_samples.front().second);
        queued_samples.pop_front();
    }
}

static LTfloat get_gain(LTObject *obj) {
    LTfloat val;
    alGetSourcef(((LTTrack*)(obj))->source_id, AL_GAIN, &val);
    check_for_errors
    return val;
}

static void set_gain(LTObject *obj, LTfloat val) {
    alSourcef(((LTTrack*)(obj))->source_id, AL_GAIN, val);
    check_for_errors
}

static LTfloat get_pitch(LTObject *obj) {
    LTfloat val;
    alGetSourcef(((LTTrack*)(obj))->source_id, AL_PITCH, &val);
    check_for_errors
    return val;
}

static void set_pitch(LTObject *obj, LTfloat val) {
    alSourcef(((LTTrack*)(obj))->source_id, AL_PITCH, val);
    check_for_errors
}

LT_REGISTER_TYPE(LTTrack, "lt.Track", "lt.SceneNode")
LT_REGISTER_PROPERTY_FLOAT(LTTrack, gain, &get_gain, &set_gain)
LT_REGISTER_PROPERTY_FLOAT(LTTrack, pitch, &get_pitch, &set_pitch)

void ltAudioSuspend() {
    if (!audio_is_suspended) {
        for (unsigned i = 0; i < sources.size(); i++) {
            Source s = sources[i];
            fixup_source_state(s.source_id);
            ALint state;
            alGetSourcei(s.source_id, AL_SOURCE_STATE, &state);
            check_for_errors
            if (state == AL_PLAYING) {
                s.play_on_resume = true;
                alSourcePause(s.source_id);
                check_for_errors
            } else {
                s.play_on_resume = false;
            }
        }
        audio_is_suspended = true;
    }
}

void ltAudioResume() {
    if (audio_is_suspended) {
        for (unsigned i = 0; i < sources.size(); i++) {
            Source s = sources[i];
            if (s.play_on_resume) {
                alSourcePlay(s.source_id);
                check_for_errors
                s.play_on_resume = false;
            }
        }
        audio_is_suspended = false;
    }
}

void ltAudioGC() {
    static int prev_num_temp = 0;
    static int prev_num_used = 0;
    static int num_temp = 0;
    static int num_used = 0;
    if (!audio_is_suspended) {
        for (unsigned i = 0; i < sources.size(); i++) {
            Source s = sources[i];
            fixup_source_state(s.source_id);
            if (!s.free) {
                if (s.temp) {
                    num_temp++;
                }
                num_used++;
            }
            if (!s.free && s.temp) {
                ALint state;
                alGetSourcei(s.source_id, AL_SOURCE_STATE, &state);
                check_for_errors
                if (state != AL_PLAYING) {
                    s.free = true;
                    alSourcei(s.source_id, AL_BUFFER, 0);
                    check_for_errors
                }
            }
        }
    }
    if (num_temp != prev_num_temp || num_used != prev_num_used) {
        fprintf(stderr, "Audio sources: used: %d, temp: %d\n", num_used, num_temp);
        prev_num_used = num_used;
        prev_num_temp = num_temp;
        num_used = 0;
        num_temp = 0;
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

LTAudioSample *ltReadAudioSample(lua_State *L, const char *path, const char *name) {
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
    
    LTAudioSample *buf = new (lt_alloc_LTAudioSample(L)) LTAudioSample(buf_id, name);
    return buf;
}
