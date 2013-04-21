#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/glfw.h>

#include "lt.h"

static void key_handler(int key, int state);
static void mouse_button_handler(int button, int action);
static void mouse_pos_handler(int x, int y);
static void resize_handler(int w, int h);
static LTKey convert_key(int key);
static bool quit = false;
static bool fullscreen = false;
static bool toggle_fullscreen = false;
static void process_args(int argc, const char **argv);
static const char *dir = "";
static const char *title = "";

static void setup_window();

int main(int argc, const char **argv) {
#ifdef LTOSX
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#endif

    process_args(argc, argv);

    #ifdef LTDEVMODE
    ltClientInit();
    #endif

    ltLuaSetResourcePrefix(dir);
    ltLuaSetup();

    if (glfwInit() != GL_TRUE) {
        fprintf(stderr, "Failed to initialize glfw. Aborting.\n");
        return 1;
    }

    setup_window();

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return 1;
    }

    if (!GLEW_VERSION_1_5)
    {
        fprintf(stderr, "Sorry, OpenGL 1.5 is required.\n");
        return 1;
    }

    bool first_time = true;
    double t0 = glfwGetTime();
    double t = t0;
    double t_debt = 0.0;
    double fps_t0 = 0.0;
    const double frame_time = 1.0 / 60.0;
    long long frame_count = 0;

    while (!quit) {
        if (!glfwGetWindowParam(GLFW_OPENED)) {
            quit = true;
        }

        if (toggle_fullscreen) {
            ltSaveState();
            glfwCloseWindow();
            fullscreen = !fullscreen;
            setup_window();
            ltLuaReset();
            toggle_fullscreen = false;
        }

        ltLuaRender();
        glfwSwapBuffers();

        // There seems to be a bug where the framerate skyrockets when the
        // window is inactive.  This works around that (except that
        // it doesn't always work, because glfwGetWindowParam returns
        // incorrect results - see below).
        if (!glfwGetWindowParam(GLFW_ACTIVE)) {
            usleep(16000);
        }

        while (t_debt > 0.0) {
            ltLuaAdvance(frame_time);
            t_debt -= frame_time;
        }

        t = glfwGetTime();
        if (!first_time) {
            double dt = fmin(0.1, t - t0); // Max of 0.1s in case process was suspended
            t_debt += dt;
            // Sleep for a bit to try and avoid the skyrocketing framerate
            // problem described above.
            if (dt < frame_time * 0.5) {
                useconds_t sleep_time = (int)((frame_time - dt) * 500000.0);
                //fprintf(stderr, "sleeping for %d\n", sleep_time);
                usleep(sleep_time);
            }

        } else {
            t_debt = frame_time;
            first_time = false;
        }
        t0 = t;

        frame_count++;
#ifdef FPS
        if (t - fps_t0 >= 2.0) {
            double fps = (double)frame_count / (t - fps_t0);
            ltLog("%0.02f fps %d objs %d actions", fps, ltNumLiveObjects(), ltNumScheduledActions());
            fps_t0 = t0;
            frame_count = 0;
        }
#endif
    #ifdef LTDEVMODE
    ltClientStep();
    #endif

    }

    ltSaveState();
    ltLuaTeardown();
    if (ltNumLiveObjects() != 0) {
        fprintf(stderr, "ERROR: num live objects not zero (%d in fact)\n", ltNumLiveObjects());
    }
    glfwTerminate();
#ifdef LTOSX
    [pool release];
#endif
    return 0;
}

#define SCALE 1

static void setup_window() {
    int w = 1138/SCALE;
    int h = 640/SCALE;
    GLFWvidmode vidmode;
    int screen_mode = GLFW_WINDOW;
    if (fullscreen) {
        screen_mode = GLFW_FULLSCREEN;
    }

    glfwGetDesktopMode(&vidmode);
    if (!fullscreen) {
        vidmode.Width = w;
        vidmode.Height = h;
    }
    //glfwOpenWindowHint(GLFW_OPENGL_PROFILE, 0);
    if (!glfwOpenWindow(vidmode.Width, vidmode.Height, vidmode.RedBits, vidmode.GreenBits, vidmode.BlueBits, 0, 16, 0, screen_mode)) {
        glfwTerminate();
        fprintf(stderr, "Failed to create window\n");
        exit(EXIT_FAILURE);
    }
    glfwSwapInterval(1);
    glfwSetWindowTitle(title);

    ltLuaResizeWindow(vidmode.Width, vidmode.Height);
    glfwSetKeyCallback(key_handler);
    glfwSetMouseButtonCallback(mouse_button_handler);
    glfwSetMousePosCallback(mouse_pos_handler);
    glfwSetWindowSizeCallback(resize_handler);
}

static void key_handler(int key, int state) {
    LTKey ltkey = convert_key(key);
    if (state == GLFW_PRESS) {
        if (key == GLFW_KEY_ESC) {
            quit = true;
        }
        if (key == 'F'
            && (glfwGetKey(GLFW_KEY_LCTRL) == GLFW_PRESS
            || glfwGetKey(GLFW_KEY_RCTRL) == GLFW_PRESS))
        {
            toggle_fullscreen = true;
        } else {
            ltLuaKeyDown(ltkey);
        }
    } else if (state == GLFW_RELEASE) {
        ltLuaKeyUp(ltkey);
    }
}

static void mouse_button_handler(int button, int action) {
    int input = 0;
    switch (button) {
        case GLFW_MOUSE_BUTTON_1: input = 1; break;
        case GLFW_MOUSE_BUTTON_2: input = 2; break;
        case GLFW_MOUSE_BUTTON_3: input = 3; break;
        case GLFW_MOUSE_BUTTON_4: input = 4; break;
        case GLFW_MOUSE_BUTTON_5: input = 5; break;
        case GLFW_MOUSE_BUTTON_6: input = 6; break;
        case GLFW_MOUSE_BUTTON_7: input = 7; break;
        case GLFW_MOUSE_BUTTON_8: input = 8; break;
    }
    int x, y;
    glfwGetMousePos(&x, &y);
    if (action == GLFW_PRESS) {
        ltLuaMouseDown(input, x, y);
    } else {
        ltLuaMouseUp(input, x, y);
    }
}

static void mouse_pos_handler(int x, int y) {
    ltLuaMouseMove(x, y);
}

static void resize_handler(int w, int h) {
    ltLuaResizeWindow(w, h);
}

static LTKey convert_key(int key) {
    switch(key) {
        case GLFW_KEY_TAB: return LT_KEY_TAB;
        case GLFW_KEY_ENTER: return LT_KEY_ENTER;
        case GLFW_KEY_ESC: return LT_KEY_ESC;
        case GLFW_KEY_SPACE: return LT_KEY_SPACE;
        case '\'': return LT_KEY_APOS;
        case '=': return LT_KEY_PLUS;
        case ',': return LT_KEY_COMMA;
        case '-': return LT_KEY_MINUS;
        case '.': return LT_KEY_PERIOD;
        case '/': return LT_KEY_SLASH;
        case '0': return LT_KEY_0;
        case '1': return LT_KEY_1;
        case '2': return LT_KEY_2;
        case '3': return LT_KEY_3;
        case '4': return LT_KEY_4;
        case '5': return LT_KEY_5;
        case '6': return LT_KEY_6;
        case '7': return LT_KEY_7;
        case '9': return LT_KEY_9;
        case ';': return LT_KEY_SEMI_COLON;
        case '[': return LT_KEY_LEFT_BRACKET;
        case '\\': return LT_KEY_BACKSLASH;
        case ']': return LT_KEY_RIGHT_BRACKET;
        case '`': return LT_KEY_TICK;
        case 'A': return LT_KEY_A;
        case 'B': return LT_KEY_B;
        case 'C': return LT_KEY_C;
        case 'D': return LT_KEY_D;
        case 'E': return LT_KEY_E;
        case 'F': return LT_KEY_F;
        case 'G': return LT_KEY_G;
        case 'H': return LT_KEY_H;
        case 'I': return LT_KEY_I;
        case 'J': return LT_KEY_J;
        case 'K': return LT_KEY_K;
        case 'L': return LT_KEY_L;
        case 'M': return LT_KEY_M;
        case 'N': return LT_KEY_N;
        case 'O': return LT_KEY_O;
        case 'P': return LT_KEY_P;
        case 'Q': return LT_KEY_Q;
        case 'R': return LT_KEY_R;
        case 'S': return LT_KEY_S;
        case 'T': return LT_KEY_T;
        case 'U': return LT_KEY_U;
        case 'V': return LT_KEY_V;
        case 'W': return LT_KEY_W;
        case 'X': return LT_KEY_X;
        case 'Y': return LT_KEY_Y;
        case 'Z': return LT_KEY_Z;
        case GLFW_KEY_DEL: return LT_KEY_DEL;
        case GLFW_KEY_UP: return LT_KEY_UP;
        case GLFW_KEY_DOWN: return LT_KEY_DOWN;
        case GLFW_KEY_RIGHT: return LT_KEY_RIGHT;
        case GLFW_KEY_LEFT: return LT_KEY_LEFT;
    }
    return LT_KEY_UNKNOWN;
}

static void process_args(int argc, const char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-fullscreen") == 0) {
            fullscreen = true;
        } else {
            dir = argv[i];
        }
    }
}
