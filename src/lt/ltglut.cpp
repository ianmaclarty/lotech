/* Copyright (C) 2010 Ian MacLarty */
#ifdef LTGLUT

extern "C" {
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
}

#include "ltcommon.h"

#ifdef LTLINUX
    #define GLX_GLXEXT_PROTOTYPES
    #include <GL/glut.h>
#else
    #include <GLUT/GLUT.h>
#endif

#include "ltgraphics.h"
#include "ltharness.h"

static bool g_fullscreen = false;
static int  g_timer_interval = 17;

static ltVoidCallback           g_setup = NULL;
static ltVoidCallback           g_teardown = NULL;
static ltVoidCallback           g_render = NULL;
static ltVoidCallback           g_advance = NULL;
static ltKeyCallback            g_key_down = NULL;
static ltKeyCallback            g_key_up = NULL;
static ltPointerCallback        g_mouse_down = NULL;
static ltPointerCallback        g_mouse_up = NULL;
static ltPointerCallback        g_mouse_move = NULL;
static ltPairCallback           g_resize_window = NULL;

static void glut_advance(int unused) {
    glutTimerFunc(g_timer_interval, glut_advance, 0);
    if (g_advance != NULL) {
        g_advance();
    }
    glutPostRedisplay();
}

static void glut_setup(int unused) {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    ltSetScreenSize(w, h);
    ltInitGraphics();
    glFlush();
    glutSwapBuffers();
    if (g_setup != NULL) {
        g_setup();
    }
    if (g_resize_window != NULL) {
        g_resize_window(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    }
    glutTimerFunc(1, glut_advance, 0);
}

static void glut_render() {
    if (g_render != NULL) {
        g_render();
    }
    glFlush();
    glutSwapBuffers();
}

static void glut_mouse_move(int x, int y) {
    if (g_mouse_move != NULL) {
        g_mouse_move(0, (LTfloat)x, (LTfloat)y);
    }
}

static void glut_mouse_button(int button, int state, int x, int y) {
    int but;
    switch (button) {
        case GLUT_LEFT_BUTTON: but = 1; break;
        case GLUT_RIGHT_BUTTON: but = 2; break;
        case GLUT_MIDDLE_BUTTON: but = 3; break;
        default: but = 0;
    }
    switch (state) {
        case GLUT_DOWN: {
            if (g_mouse_down != NULL) {
                g_mouse_down(but, (LTfloat)x, (LTfloat)y);
            }
            break;
        }
        case GLUT_UP: {
            if (g_mouse_up != NULL) {
                g_mouse_up(but, (LTfloat)x, (LTfloat)y);
            }
            break;
        }
    }
}

static void glut_resize_window(int w, int h) {
    if (g_resize_window != NULL) {
        g_resize_window((LTfloat)w, (LTfloat)h);
    }
}

static LTKey glut_special_key_to_lt_key(int key) {
    switch (key) {
        case GLUT_KEY_LEFT: return LT_KEY_LEFT;
        case GLUT_KEY_RIGHT: return LT_KEY_RIGHT;
        case GLUT_KEY_UP: return LT_KEY_UP;
        case GLUT_KEY_DOWN: return LT_KEY_DOWN;
        default: return LT_KEY_UNKNOWN;
    }
}

static LTKey glut_key_to_lt_key(unsigned char key) {
    switch (key) {
        case 'a':
        case 'A':
            return LT_KEY_A;
        case 'b':
        case 'B':
            return LT_KEY_B;
        case 'c':
        case 'C':
            return LT_KEY_C;
        case 'd':
        case 'D':
            return LT_KEY_D;
        case 'e':
        case 'E':
            return LT_KEY_E;
        case 'f':
        case 'F':
            return LT_KEY_F;
        case 'g':
        case 'G':
            return LT_KEY_G;
        case 'h':
        case 'H':
            return LT_KEY_H;
        case 'i':
        case 'I':
            return LT_KEY_I;
        case 'j':
        case 'J':
            return LT_KEY_J;
        case 'k':
        case 'K':
            return LT_KEY_K;
        case 'l':
        case 'L':
            return LT_KEY_L;
        case 'm':
        case 'M':
            return LT_KEY_M;
        case 'n':
        case 'N':
            return LT_KEY_N;
        case 'o':
        case 'O':
            return LT_KEY_O;
        case 'p':
        case 'P':
            return LT_KEY_P;
        case 'q':
        case 'Q':
            return LT_KEY_Q;
        case 'r':
        case 'R':
            return LT_KEY_R;
        case 's':
        case 'S':
            return LT_KEY_S;
        case 't':
        case 'T':
            return LT_KEY_T;
        case 'u':
        case 'U':
            return LT_KEY_U;
        case 'v':
        case 'V':
            return LT_KEY_V;
        case 'w':
        case 'W':
            return LT_KEY_W;
        case 'x':
        case 'X':
            return LT_KEY_X;
        case 'y':
        case 'Y':
            return LT_KEY_Y;
        case 'z':
        case 'Z':
            return LT_KEY_Z;
        case '0':
            return LT_KEY_0;
        case '1':
            return LT_KEY_1;
        case '2':
            return LT_KEY_2;
        case '3':
            return LT_KEY_3;
        case '4':
            return LT_KEY_4;
        case '5':
            return LT_KEY_5;
        case '6':
            return LT_KEY_6;
        case '7':
            return LT_KEY_7;
        case '8':
            return LT_KEY_8;
        case '9':
            return LT_KEY_9;
        case ' ':
            return LT_KEY_SPACE;
        case 13:
            return LT_KEY_ENTER;
        case 127:
            return LT_KEY_DEL;
        default:
            fprintf(stderr, "Unknown key pressed: %c (%d)\n", key, key);
            return LT_KEY_UNKNOWN;
    }
}

static void glut_key_up(unsigned char key, int x, int y) {
    if (g_key_up != NULL) {
        LTKey ltkey = glut_key_to_lt_key(key);
        g_key_up(ltkey);
    }
}

static void glut_key_down(unsigned char key, int x, int y) {
    if (g_key_down != NULL) {
        LTKey ltkey = glut_key_to_lt_key(key);
        g_key_down(ltkey);
    }
}

static void glut_special_key_up(int key, int x, int y) {
    if (g_key_up != NULL) {
        LTKey ltkey = glut_special_key_to_lt_key(key);
        g_key_up(ltkey);
    }
}

static void glut_special_key_down(int key, int x, int y) {
    if (g_key_down != NULL) {
        LTKey ltkey = glut_special_key_to_lt_key(key);
        g_key_down(ltkey);
    }
}

void ltHarnessInit(bool fullscreen, const char *title, int fps,
    ltVoidCallback setup, ltVoidCallback teardown,
    ltVoidCallback render, ltVoidCallback advance,
    ltKeyCallback keyDown, ltKeyCallback keyUp,
    ltPointerCallback mouseDown, ltPointerCallback mouseUp,
    ltPointerCallback mouseMove,
    ltPairCallback resizeWindow)
{
    int argc = 0;
    char *argv = NULL;

    g_fullscreen = fullscreen;
    g_timer_interval = (int)floor((1.0 / (double)fps) * 1000.0);
    g_setup = setup;
    g_teardown = teardown;
    g_render = render;
    g_advance = advance;
    g_key_down = keyDown;
    g_key_up = keyUp;
    g_mouse_down = mouseDown;
    g_mouse_up = mouseUp;
    g_mouse_move = mouseMove;
    g_resize_window = resizeWindow;

    glutInitWindowSize(1024, 768);
    glutInit(&argc, &argv);
    unsigned int mode = GLUT_DOUBLE | GLUT_RGBA;
    #ifdef LTDEPTHBUF
    mode |= GLUT_DEPTH;
    #endif
    glutInitDisplayMode(mode);
    if (g_fullscreen) {
        glutGameModeString("1680x1050:32@60");
        glutEnterGameMode();
    } else {
        glutCreateWindow(title);
    }

    glutDisplayFunc(glut_render);
    glutMotionFunc(glut_mouse_move);
    glutPassiveMotionFunc(glut_mouse_move);
    glutReshapeFunc(glut_resize_window);
    glutMouseFunc(glut_mouse_button);
    glutKeyboardFunc(glut_key_down);
    glutKeyboardUpFunc(glut_key_up);
    glutSpecialFunc(glut_special_key_down);
    glutSpecialUpFunc(glut_special_key_up);
    glutIgnoreKeyRepeat(1);
    glutTimerFunc(1, glut_setup, 0);
    glutMainLoop();
}

void ltHarnessQuit() {
    if (g_teardown != NULL) {
        g_teardown();
    }
    if (g_fullscreen) {
        glutLeaveGameMode();
    }
    exit(0);
}

#endif /* LTGLUT */
