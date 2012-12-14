/* Copyright (C) 2012 Ian MacLarty */

#include "lt.h"

struct thread_data {
    void          (*f)(void*);
    void *        data;
    pthread_t     thread;
};

static void *wrap(void *ud) {
    thread_data *d = (thread_data*)ud;
    d->f(d->data);
    return NULL;
}

void ltNewThread(void (*f)(void *), void* data) {
    thread_data *d = new thread_data(); // XXX never deleted
    d->f = f;
    d->data = data;
    pthread_create(&d->thread, NULL, wrap, d);
}

LTMutex  *ltCreateMutex() {
    pthread_mutex_t *mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);
    return mutex;
}

void ltDeleteMutex(LTMutex *mutex) {
    pthread_mutex_destroy(mutex);
    free(mutex);
}

void ltLockMutex(LTMutex *mutex) {
    pthread_mutex_lock(mutex);
}

void ltUnlockMutex(LTMutex *mutex) {
    pthread_mutex_unlock(mutex);
}
