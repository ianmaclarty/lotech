/* Copyright (C) 2010-2011 Ian MacLarty */
#ifndef LTAUDIO_H
#define LTAUDIO_H

#include <stdlib.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include "ltcommon.h"

void ltAudioInit();
void ltAudioTeardown();

struct LTAudioSample : LTObject {
    ALuint buffer_id;
    char *name;

    // name is copied.
    LTAudioSample(ALuint buffer_id, const char *name);
    virtual ~LTAudioSample();

    // Create a new source, play it, and delete the source.
    // Requires ltAudioGC() to be called after audio has finished playing.
    void play(LTfloat pitch = 1.0f, LTfloat gain = 1.0f);
};

struct LTTrack : LTObject {
    ALuint source_id;

    LTTrack();
    virtual ~LTTrack();

    void queueSample(LTAudioSample *sample);
    void setLoop(bool loop);
    void play();
    int  numSamples();
    int  numProcessedSamples();
    int  numPendingSamples(); // numSamples() - numProcessedSamples()
    void dequeueProcessedSamples(int n);

    virtual bool has_field(const char *field_name);
    virtual LTfloat get_field(const char *field_name);
    virtual void set_field(const char *field_name, LTfloat value);
};

// name is copied.
LTAudioSample *ltReadAudioSample(const char *path, const char *name);

// Collect temporary sources created for oneoff buffer playing.
void ltAudioGC();

#endif
