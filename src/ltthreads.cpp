/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"
#include <pthread.h>

struct thread_data {
    void          (*f)(void*);
    void *        data;
    pthread_t     thread_id;
};

static void *wrap(void *ud) {
    thread_data *d = (thread_data*)ud;
    d->f(d->data);
    delete d;
    return NULL;
}

void ltNewThread(void (*f)(void *), void* data) {
    thread_data *d = new thread_data();
    d->f = f;
    d->data = data;
    pthread_create(&d->thread_id, NULL, wrap, d);
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
