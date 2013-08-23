/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltconfig)

const char *lt_app_short_name = NULL;
bool lt_vsync = true;
bool lt_fullscreen = false;
bool lt_show_mouse_cursor = true;
bool lt_quit = false;
bool lt_letterbox = false;
double lt_fixed_update_time = 1.0/60.0;
