#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#include "lt.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

static bool did_resize_screen = false;

static LTKey to_lt_key(int key);

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;
    JNIEnv *jni;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    bool initialized;
    bool first_time;
    int32_t width;
    int32_t height;
};

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    //ltLog("===init display===");
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
    };
    EGLint w, h, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->animating = 1;

    ltLuaReset();
    if (!did_resize_screen && w > 0 && h > 0) {
        ltResizeScreen(w, h);
        did_resize_screen = true;
    }
    engine->initialized = true;
    engine->first_time = true;
    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) {
        // No display.
        return;
    }

    if (engine->initialized) {
        //ltLog("===render===");
        ltLuaRender();
    }
    eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    ltLuaTeardown();
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
    engine->initialized = false;
}

enum button {
    D_LEFT,
    D_RIGHT,
    D_UP,
    D_DOWN,
    LS_LEFT,
    LS_RIGHT,
    LS_UP,
    LS_DOWN,
};

static bool button_down[64];

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int32_t actioncode = AMotionEvent_getAction(event);
        int action = AMOTION_EVENT_ACTION_MASK & actioncode;
        int pointer_index = (actioncode & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
            >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        int pointer_id = AMotionEvent_getPointerId(event, pointer_index);

        float v;

        v = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_X, pointer_index);
        if (v < -0.5) {
            if (button_down[D_RIGHT]) {
                button_down[D_RIGHT] = false;
                ltLuaKeyUp(LT_KEY_RIGHT);
            }
            if (!button_down[D_LEFT]) {
                button_down[D_LEFT] = true;
                ltLuaKeyDown(LT_KEY_LEFT);
            }
        } else if (v > 0.5) {
            if (button_down[D_LEFT]) {
                button_down[D_LEFT] = false;
                ltLuaKeyUp(LT_KEY_LEFT);
            }
            if (!button_down[D_RIGHT]) {
                button_down[D_RIGHT] = true;
                ltLuaKeyDown(LT_KEY_RIGHT);
            }
        } else {
            if (button_down[D_LEFT]) {
                button_down[D_LEFT] = false;
                ltLuaKeyUp(LT_KEY_LEFT);
            }
            if (button_down[D_RIGHT]) {
                button_down[D_RIGHT] = false;
                ltLuaKeyUp(LT_KEY_RIGHT);
            }
        }

        v = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_Y, pointer_index);
        if (v < -0.5) {
            if (button_down[D_DOWN]) {
                button_down[D_DOWN] = false;
                ltLuaKeyUp(LT_KEY_DOWN);
            }
            if (!button_down[D_UP]) {
                button_down[D_UP] = true;
                ltLuaKeyDown(LT_KEY_UP);
            }
        } else if (v > 0.5) {
            if (button_down[D_UP]) {
                button_down[D_UP] = false;
                ltLuaKeyUp(LT_KEY_UP);
            }
            if (!button_down[D_DOWN]) {
                button_down[D_DOWN] = true;
                ltLuaKeyDown(LT_KEY_DOWN);
            }
        } else {
            if (button_down[D_UP]) {
                button_down[D_UP] = false;
                ltLuaKeyUp(LT_KEY_UP);
            }
            if (button_down[D_DOWN]) {
                button_down[D_DOWN] = false;
                ltLuaKeyUp(LT_KEY_DOWN);
            }
        }

        v = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_X, pointer_index);
        if (v < -0.5) {
            if (button_down[LS_RIGHT]) {
                button_down[LS_RIGHT] = false;
                ltLuaKeyUp(LT_KEY_RIGHT);
            }
            if (!button_down[LS_LEFT]) {
                button_down[LS_LEFT] = true;
                ltLuaKeyDown(LT_KEY_LEFT);
            }
        } else if (v > 0.5) {
            if (button_down[LS_LEFT]) {
                button_down[LS_LEFT] = false;
                ltLuaKeyUp(LT_KEY_LEFT);
            }
            if (!button_down[LS_RIGHT]) {
                button_down[LS_RIGHT] = true;
                ltLuaKeyDown(LT_KEY_RIGHT);
            }
        } else {
            if (button_down[LS_LEFT]) {
                button_down[LS_LEFT] = false;
                ltLuaKeyUp(LT_KEY_LEFT);
            }
            if (button_down[LS_RIGHT]) {
                button_down[LS_RIGHT] = false;
                ltLuaKeyUp(LT_KEY_RIGHT);
            }
        }

        v = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Y, pointer_index);
        if (v < -0.5) {
            if (button_down[LS_DOWN]) {
                button_down[LS_DOWN] = false;
                ltLuaKeyUp(LT_KEY_DOWN);
            }
            if (!button_down[LS_UP]) {
                button_down[LS_UP] = true;
                ltLuaKeyDown(LT_KEY_UP);
            }
        } else if (v > 0.5) {
            if (button_down[LS_UP]) {
                button_down[LS_UP] = false;
                ltLuaKeyUp(LT_KEY_UP);
            }
            if (!button_down[LS_DOWN]) {
                button_down[LS_DOWN] = true;
                ltLuaKeyDown(LT_KEY_DOWN);
            }
        } else {
            if (button_down[LS_UP]) {
                button_down[LS_UP] = false;
                ltLuaKeyUp(LT_KEY_UP);
            }
            if (button_down[LS_DOWN]) {
                button_down[LS_DOWN] = false;
                ltLuaKeyUp(LT_KEY_DOWN);
            }
        }

        /*
        float x = AMotionEvent_getX(event, pointer_index);
        float y = AMotionEvent_getY(event, pointer_index);
        if (action == AMOTION_EVENT_ACTION_POINTER_DOWN
            || action == AMOTION_EVENT_ACTION_DOWN)
        {
            ltLuaTouchDown(pointer_id, x, y);
        } else if (action == AMOTION_EVENT_ACTION_POINTER_UP
            || action == AMOTION_EVENT_ACTION_UP
            || action == AMOTION_EVENT_ACTION_CANCEL)
        {
            ltLuaTouchUp(pointer_id, x, y);
        } else if (action == AMOTION_EVENT_ACTION_MOVE
            || action == AMOTION_EVENT_ACTION_OUTSIDE)
        {
            ltLuaTouchMove(pointer_id, x, y);
        }
        ltLog("motion: %d", action);
        engine->animating = 1;
        */
        return 1;
    } else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
        int key = AKeyEvent_getKeyCode(event);
        int32_t actioncode = AKeyEvent_getAction(event);
        int32_t repeats = AKeyEvent_getRepeatCount(event);
        //ltLog("KEY: %d %d (%d)", key, actioncode, repeats);
        LTKey ltkey = to_lt_key(key);
        if (ltkey == LT_KEY_UNKNOWN) {
            return 0; // use default key handler.
        } else if (repeats == 0) {
            switch (actioncode) {
                case AKEY_EVENT_ACTION_DOWN: ltLuaKeyDown(ltkey); break;
                case AKEY_EVENT_ACTION_UP: ltLuaKeyUp(ltkey); break;
            }
            // Do default behaviour if lt.Quit() called.
            // This will do what we expect if the key pressed was the back
            // key. We handle quitting this way, because calling
            //   ANativeActivity_finish(engine.app->activity);
            // from the main loop does not have the desired effect -
            // it quits the app, but then when you try to start it again
            // you get a "Duplicate finish request for HistoryRecord" error.
            if (lt_quit) return 0; 
        }
        return 1;
    }
    return 0; // use default key handler.
}

static void quit(struct android_app *state) {
    JNIEnv* jni;
    state->activity->vm->AttachCurrentThread(&jni, NULL);

    jclass activityClass = jni->GetObjectClass(state->activity->clazz);
    jmethodID quitMethod = jni->GetMethodID(activityClass, "quit", "()V");
    jobject pathObject = jni->CallObjectMethod(state->activity->clazz, quitMethod);

    jni->DeleteLocalRef(activityClass);

    state->activity->vm->DetachCurrentThread();
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            ltLog("APP_CMD_SAVE_STATE");
            ltSaveState();
            break;
        case APP_CMD_INIT_WINDOW:
            ltLog("APP_CMD_INIT_WINDOW");
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {
                engine_init_display(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            ltLog("APP_CMD_TERM_WINDOW");
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            // Quit the app. We will need to restart anyway when we
            // re-initialise the display. Do a hard restart
            // seems to be more reliable the re-initializing
            // with an already running app.
            quit(app);
            break;
        case APP_CMD_GAINED_FOCUS:
            ltLog("APP_CMD_GAINED_FOCUS");
            engine->animating = 1;
            break;
        case APP_CMD_LOST_FOCUS:
            ltLog("APP_CMD_LOST_FOCUS");
            engine->animating = 0;
            break;
        case APP_CMD_WINDOW_RESIZED: {
            /*
            int w = ANativeWindow_getWidth(engine->app->window);
            int h = ANativeWindow_getHeight(engine->app->window);
            ltLog("APP_CMD_WINDOW_RESIZED %d %d", w, h);
            */
            ltLog("APP_CMD_WINDOW_RESIZED");
            break;
        }
        case APP_CMD_CONTENT_RECT_CHANGED: {
            /*
            ltLog("APP_CMD_CONTENT_RECT_CHANGED %d %d %d %d",
                engine->app->contentRect.left,
                engine->app->contentRect.right,
                engine->app->contentRect.top,
                engine->app->contentRect.bottom);
            */
            ltLog("APP_CMD_CONTENT_RECT_CHANGED");
            break;
        }
        case APP_CMD_CONFIG_CHANGED: {
            /*
            int w = ANativeWindow_getWidth(engine->app->window);
            int h = ANativeWindow_getHeight(engine->app->window);
            ltLog("APP_CMD_CONFIG_CHANGED %d %d", w, h);
            */
            ltLog("APP_CMD_CONFIG_CHANGED");
            //ltResizeScreen(w, h);
            break;
        }
        case APP_CMD_LOW_MEMORY: {
            ltLog("APP_CMD_LOW_MEMORY");
            ltLuaGarbageCollect();
            break;
        }
        case APP_CMD_INPUT_CHANGED:
            ltLog("APP_CMD_INPUT_CHANGED");
            break;
        case APP_CMD_WINDOW_REDRAW_NEEDED:
            ltLog("APP_CMD_WINDOW_REDRAW_NEEDED");
            break;
        case APP_CMD_START:
            ltLog("APP_CMD_START");
            break;
        case APP_CMD_RESUME:
            ltLog("APP_CMD_RESUME");
            break;
        case APP_CMD_PAUSE:
            ltLog("APP_CMD_PAUSE");
            break;
        case APP_CMD_STOP:
            ltLog("APP_CMD_STOP");
            break;
        case APP_CMD_DESTROY:
            ltLog("APP_CMD_DESTROY");
            break;
    }
}

static double get_time() {
    struct timespec tm;
    clock_gettime(CLOCK_MONOTONIC, &tm);
    double t = 0.000000001 * (double)tm.tv_nsec + (double)tm.tv_sec;
    return t;
}

static void set_data_dir(struct android_app *state) {
    lt_android_data_dir = state->activity->internalDataPath;
    if (lt_android_data_dir == NULL) {
        JNIEnv* jni;
        state->activity->vm->AttachCurrentThread(&jni, NULL);

        jclass activityClass = jni->GetObjectClass(state->activity->clazz);
        jmethodID getFilesDir = jni->GetMethodID(activityClass, "getFilesDir", "()Ljava/io/File;");
        jobject fileObject = jni->CallObjectMethod(state->activity->clazz, getFilesDir);
        jclass fileClass = jni->GetObjectClass(fileObject);
        jmethodID getAbsolutePath = jni->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
        jobject pathObject = jni->CallObjectMethod(fileObject, getAbsolutePath);
        lt_android_data_dir = jni->GetStringUTFChars((jstring)pathObject, NULL);

        jni->DeleteLocalRef(pathObject);
        jni->DeleteLocalRef(fileClass);
        jni->DeleteLocalRef(fileObject);
        jni->DeleteLocalRef(activityClass);

        state->activity->vm->DetachCurrentThread();
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    double t0, t;
    double fps_t = 0.0;
    int frames = 0;
    double t_debt = 0.0;
    double t_debt_payment = 1.0/60.0;
    struct engine engine;

    for (int i = 0; i < 64; i++) {
        button_down[i] = false;
    }

    //ltLog("*********************************   starting   **********************************");

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    engine.initialized = false;
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    //ltLog("===set data dir===");
    set_data_dir(state);

    //ltLog("===set asset manager===");
    ltSetAssetManager(state->activity->assetManager);

    // loop waiting for stuff to do.
    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        //ltLog("===main loop===");

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                (void**)&source)) >= 0) {

            //ltLog("===start input poll loop===");

            // Process this event.
            if (source != NULL) {
                //ltLog("===processing event===");
                source->process(state, source);
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                //ltLog("===destroying===");
                engine_term_display(&engine);
                return;
            }

            //ltLog("===end input poll loop===");
        }

        if (engine.initialized && engine.animating) {
            engine_draw_frame(&engine);

            t = get_time();
            if (engine.first_time) {
                t0 = t;
                engine.first_time = false;
            } else {
                t_debt += t - t0;
                //ltLog("t_debt = %f, t = %f, t0 = %f", t_debt, t, t0);
                if (t_debt > 0.1) t_debt = 0.1; 
                t0 = t;
            }
            while (t_debt > 0.0) {
                //ltLog("===advance===");
                ltLuaAdvance((float)t_debt_payment);
                t_debt -= t_debt_payment;
            }

            frames++;
            if (t > fps_t) {
                ltLog("FPS: %d", frames);
                frames = 0;
                fps_t = t + 1.0;
            }
        }
    }
}

static LTKey to_lt_key(int key) {
    switch (key) {
       case AKEYCODE_BACK: return LT_KEY_ESC;

       case AKEYCODE_0: return LT_KEY_0;
       case AKEYCODE_1: return LT_KEY_1;
       case AKEYCODE_2: return LT_KEY_2;
       case AKEYCODE_3: return LT_KEY_3;
       case AKEYCODE_4: return LT_KEY_4;
       case AKEYCODE_5: return LT_KEY_5;
       case AKEYCODE_6: return LT_KEY_6;
       case AKEYCODE_7: return LT_KEY_7;
       case AKEYCODE_8: return LT_KEY_8;
       case AKEYCODE_9: return LT_KEY_9;

       case AKEYCODE_A: return LT_KEY_A;
       case AKEYCODE_B: return LT_KEY_B;
       case AKEYCODE_C: return LT_KEY_C;
       case AKEYCODE_D: return LT_KEY_D;
       case AKEYCODE_E: return LT_KEY_E;
       case AKEYCODE_F: return LT_KEY_F;
       case AKEYCODE_G: return LT_KEY_G;
       case AKEYCODE_H: return LT_KEY_H;
       case AKEYCODE_I: return LT_KEY_I;
       case AKEYCODE_J: return LT_KEY_J;
       case AKEYCODE_K: return LT_KEY_K;
       case AKEYCODE_L: return LT_KEY_L;
       case AKEYCODE_M: return LT_KEY_M;
       case AKEYCODE_N: return LT_KEY_N;
       case AKEYCODE_O: return LT_KEY_O;
       case AKEYCODE_P: return LT_KEY_P;
       case AKEYCODE_Q: return LT_KEY_Q;
       case AKEYCODE_R: return LT_KEY_R;
       case AKEYCODE_S: return LT_KEY_S;
       case AKEYCODE_T: return LT_KEY_T;
       case AKEYCODE_U: return LT_KEY_U;
       case AKEYCODE_V: return LT_KEY_V;
       case AKEYCODE_W: return LT_KEY_W;
       case AKEYCODE_X: return LT_KEY_X;
       case AKEYCODE_Y: return LT_KEY_Y;
       case AKEYCODE_Z: return LT_KEY_Z;

       case AKEYCODE_BUTTON_L1: return LT_KEY_LEFT;
       case AKEYCODE_BUTTON_R1: return LT_KEY_RIGHT;
       case AKEYCODE_BUTTON_A: return LT_KEY_ESC;
       case AKEYCODE_BUTTON_X: return LT_KEY_ENTER;

       default: return LT_KEY_UNKNOWN;
    }
}


