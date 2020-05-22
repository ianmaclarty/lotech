/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

#ifdef LTANDROID

#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>

static AAssetManager *asset_manager = NULL;
static bool android_running = false;
static bool first_time = true;

static double t0, t;
static double t_debt = 0.0;
static double t_debt_payment = 1.0/60.0;

static int win_dummy = 0;

static JNIEnv *jni_env = NULL;

static double get_time() {
    struct timespec tm;
    clock_gettime(CLOCK_MONOTONIC, &tm);
    double t = 0.000000001 * (double)tm.tv_nsec + (double)tm.tv_sec;
    return t;
}

static void android_init_engine() {
    first_time = true;
    t0 = get_time();
    ltLuaReset();
    android_running = true;
    ltLog("%s", "android_init_engine");
}

static void android_teardown() {
    ltLuaTeardown();
    android_running = false;
    ltLog("%s", "android_teardown");
}

static void android_draw() {
    if (!android_running) return;
    ltLuaRender();
    //ltLog("%s", "android_draw");
}

static void android_update() {
    if (!android_running) return;

    t = get_time();
    if (first_time) {
        t0 = t;
        first_time = false;
    } else {
        t_debt += t - t0;
        //ltLog("t_debt = %f, t = %f, t0 = %f", t_debt, t, t0);
        if (t_debt > 0.1) t_debt = 0.1; 
        t0 = t;
    }
    while (t_debt > 0.0) {
        //ltLog("%s(%f)", "android_update", t_debt_payment);
        //ltLog("===advance===");
        ltLuaAdvance((float)t_debt_payment);
        t_debt -= t_debt_payment;
    }
}

extern "C" {
    JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniResize(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniStep(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniInit(JNIEnv * env, jobject obj, jobject jassman, jstring jdatadir, jstring jlang);
    JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniSurfaceCreated(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniTeardown(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniPause(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniResume(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniTouchDown(JNIEnv * env, jobject obj, jint id, jfloat x, jfloat y);
    JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniTouchUp(JNIEnv * env, jobject obj, jint id, jfloat x, jfloat y);
    JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniTouchMove(JNIEnv * env, jobject obj, jint id, jfloat x, jfloat y);
};

JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniResize(JNIEnv * env, jobject obj,  jint width, jint height)
{
    ltLuaResizeWindow((float)width, (float)height);

}

JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniStep(JNIEnv * env, jobject obj)
{
    jni_env = env;
    android_draw();
    android_update();
    jni_env = NULL;
}

JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniInit(JNIEnv * env, jobject obj, jobject jassman, jstring jdatadir, jstring jlang)
{
    jni_env = env;
    ltSetAssetManager(AAssetManager_fromJava(env, jassman));

    const char* datadir_tmp = env->GetStringUTFChars(jdatadir , NULL ) ;
    int len = strlen(datadir_tmp);
    lt_android_data_dir = (char*)malloc(len + 1);
    strcpy(lt_android_data_dir, datadir_tmp);
    env->ReleaseStringUTFChars(jdatadir, datadir_tmp);

    jni_env = NULL;
}

JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniSurfaceCreated(JNIEnv * env, jobject obj)
{
    jni_env = env;
    android_init_engine();
    jni_env = NULL;
}

JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniTeardown(JNIEnv * env, jobject obj) {
    android_teardown();
}

JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniTouchDown(JNIEnv * env, jobject obj, jint id, jfloat x, jfloat y) {
    jni_env = env;
    ltLuaTouchDown((int)id, x, y);
    jni_env = NULL;
}

JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniTouchUp(JNIEnv * env, jobject obj, jint id, jfloat x, jfloat y) {
    jni_env = env;
    ltLuaTouchUp((int)id, x, y);
    jni_env = NULL;
}

JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniTouchMove(JNIEnv * env, jobject obj, jint id, jfloat x, jfloat y) {
    jni_env = env;
    ltLuaTouchMove((int)id, x, y);
    jni_env = NULL;
}

JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniPause(JNIEnv * env, jobject obj)
{
}

JNIEXPORT void JNICALL Java_xyz_amulet_AmuletActivity_jniResume(JNIEnv * env, jobject obj)
{
}

#endif // LTANDROID
