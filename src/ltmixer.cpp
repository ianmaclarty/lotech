/* Copyright (C) 2012 Ian MacLarty */

#include "lt.h"

#define MIXER_SLEEP_MILLIS 10

struct LTMixer {
    LTshort *buf;
    int size;

    LTMixer(int n) {
        buf = new LTshort[n];
        memset(buf, 0, sizeof(LTshort) * n);
        size = n;
    }
    virtual ~LTMixer() {
        delete buf;
    }

    void update() {
    }
};

/*
static std::vector<LTMixer*> mixers;

static LTMutex *mixers_mutex;

static void update_mixers(void *ud) {
    do {
        ltLockMutex(mixers_mutex);
        for (unsigned i = 0; i < mixers.size(); i++) {
            mixers[i]->update();
        }
        ltUnlockMutex(mixers_mutex);
        ltSleep(MIXER_SLEEP_MILLIS);
    } while (1);
}
*/

void ltmixer_init() {
    //ltNewThread(update_mixers, NULL);
    //mixers_mutex = ltCreateMutex();
}
