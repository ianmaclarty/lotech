/* Copyright (C) 2010-2011 Ian MacLarty */
#include <errno.h>
#include <string.h>
#include <list>

#include "ltaudio.h"

#include "ltutil.h"

static ALCcontext* audio_context = NULL;
static ALCdevice* audio_device = NULL;

static std::list<ALuint> temp_sources;
static bool mute = false;

void ltAudioInit() {
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
    if (!mute) {
        ALuint source_id;
        alGenSources(1, &source_id);
        alSourcei(source_id, AL_BUFFER, buffer_id);
        alSourcef(source_id, AL_PITCH, pitch);
        alSourcef(source_id, AL_GAIN, gain);
        alSourcei(source_id, AL_LOOPING, AL_FALSE);
        alSourcePlay(source_id);
        temp_sources.push_back(source_id);
    }
}

LTTrack::LTTrack() : LTObject(LT_TYPE_TRACK) {
    alGenSources(1, &source_id);
    alSourcef(source_id, AL_PITCH, 1.0f);
    alSourcef(source_id, AL_GAIN, 1.0f);
    alSourcei(source_id, AL_LOOPING, AL_FALSE);
}

LTTrack::~LTTrack() {
    alDeleteSources(1, &source_id);
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

static bool delete_source_if_finished(ALuint source_id) {
    ALint state;
    alGetSourcei(source_id, AL_SOURCE_STATE, &state);
    if (!mute && state == AL_PLAYING) {
        return false;
    } else {
        alDeleteSources(1, &source_id);
        return true;
    }
}

void ltAudioGC() {
    temp_sources.remove_if(delete_source_if_finished);
}

static int read_4_byte_little_endian_int(FILE *file) {
    int val = 0;
    fread(&val, 1, 4, file);
    return val;
}

static int read_2_byte_little_endian_int(FILE *file) {
    int val = 0;
    fread(&val, 1, 2, file);
    return val;
}

LTAudioSample *ltReadAudioSample(const char *path, const char *name) {
    char chunkid[5];
    memset(chunkid, 0, 5);

    FILE *in = fopen(path, "r");
    if (in == NULL) {
        ltLog("Unable to open %s: %s", path, strerror(errno));
        fclose(in);
        return NULL;
    }

    fread(chunkid, 1, 4, in);
    if (strcmp(chunkid, "RIFF") != 0) {
        ltLog("RIFF chunk id not found in %s", path);
        fclose(in);
        return NULL;
    }

    read_4_byte_little_endian_int(in); // file size

    fread(chunkid, 1, 4, in);

    if (strcmp(chunkid, "WAVE") != 0) {
        ltLog("WAVE format id not found in %s", path);
        fclose(in);
        return NULL;
    }

    fread(chunkid, 1, 4, in);
    if (strcmp(chunkid, "fmt ") != 0) {
        ltLog("fmt chunk id not found in %s", path);
        fclose(in);
        return NULL;
    }

    int chunksize = read_4_byte_little_endian_int(in);
    if (chunksize != 16) {
        ltLog("fmt chunk size is not 16");
        fclose(in);
        return NULL;
    }

    int format_id = read_2_byte_little_endian_int(in);
    if (format_id != 1) {
        ltLog("Format id should be 1 for PCM");
        fclose(in);
        return NULL;
    }

    int num_channels = read_2_byte_little_endian_int(in);
    if (num_channels != 1 && num_channels != 2) {
        ltLog("Unsupported number of channels: %d in %s", num_channels, path);
        fclose(in);
        return NULL;
    }

    int sample_rate = read_4_byte_little_endian_int(in);

    read_4_byte_little_endian_int(in); // byte rate
    read_2_byte_little_endian_int(in); // bytes per sample (all channels)

    int bits_per_sample = read_2_byte_little_endian_int(in);
    if (bits_per_sample != 8 && bits_per_sample != 16) {
        ltLog("Unsupported bits per sample %d in %s", bits_per_sample, path);
        fclose(in);
        return NULL;
    }

    int bytes_per_sample = bits_per_sample / 8;

    fread(chunkid, 1, 4, in);
    if (strcmp(chunkid, "data") != 0) {
        ltLog("Data chunk not found in %s", path);
        fclose(in);
        return NULL;
    }

    int data_size = read_4_byte_little_endian_int(in);

    unsigned char *data = new unsigned char[data_size];

    int num_bytes_read = fread(data, 1, data_size, in);

    if (num_bytes_read != data_size) {
        ltLog("Unable to read all bytes in %s: %s", path, strerror(errno));
        fclose(in);
        delete[] data;
        return NULL;
    }

    unsigned char trailing;
    num_bytes_read = fread(&trailing, 1, 1, in);
    if (num_bytes_read != 0 || !feof(in)) {
        ltLog("Extra bytes at end of %s", path);
        fclose(in);
        delete[] data;
        return NULL;
    }

    fclose(in);

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
        ltLog("alBufferData returned an error");
        return NULL;
    }
    
    LTAudioSample *buf = new LTAudioSample(buf_id, name);
    return buf;
}
