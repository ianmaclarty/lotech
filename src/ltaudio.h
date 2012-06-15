/* Copyright (C) 2010-2011 Ian MacLarty */
LT_INIT_DECL(ltaudio)
void ltAudioInit();
void ltAudioTeardown();
void ltAudioSuspend();
void ltAudioResume();

struct LTAudioSample : LTObject {
    ALuint buffer_id;
    char *name;

    LTAudioSample() {
        ltLog("Don't create a sample directly. Use lt.LoadSamples instead.");
        ltAbort();
    };
    // name is copied.
    LTAudioSample(ALuint buffer_id, const char *name);
    virtual ~LTAudioSample();

    int bytes();
    int bitsPerDataPoint();
    int channels();
    int numDataPoints();
    int dataPointsPerSec();
    LTdouble length(); // In secs.

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
    void pause();
    void stop();
    void rewind();
    int  numSamples();
    int  numProcessedSamples();
    int  numPendingSamples(); // numSamples() - numProcessedSamples()
    void dequeueProcessedSamples(int n);
};

// name is copied.
LTAudioSample *ltReadAudioSample(lua_State *L, const char *path, const char *name);

// Collect temporary sources created for oneoff buffer playing.
void ltAudioGC();

LTAudioSample *lt_expect_LTAudioSample(lua_State *L, int arg);
LTTrack *lt_expect_LTTrack(lua_State *L, int arg);
