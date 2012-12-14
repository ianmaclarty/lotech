/* Copyright (C) 2012 Ian MacLarty */

typedef pthread_mutex_t LTMutex;

void ltNewThread(void f(void *), void *data);
LTMutex *ltCreateMutex();
void ltDeleteMutex(LTMutex *mutex);
void ltLockMutex(LTMutex *mutex);
void ltUnlockMutex(LTMutex *mutex);
