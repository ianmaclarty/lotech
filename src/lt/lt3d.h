/* Copyright (C) 2011 Ian MacLarty */
#ifndef LT3D_H
#define LT3D_H

#include "ltcommon.h"
#include "ltscene.h"

struct LTCuboidNode : LTSceneNode {
    LTvertbuf vertbuf;
    LTvertbuf frontbuf;
    LTvertbuf backbuf;
    LTvertbuf topbuf;
    LTvertbuf bottombuf;
    LTvertbuf leftbuf;
    LTvertbuf rightbuf;

    LTCuboidNode(LTfloat width, LTfloat height, LTfloat depth);
    virtual ~LTCuboidNode();

    virtual void draw();
};

struct LTPerspective : LTWrapNode {
    LTfloat near;
    LTfloat origin;
    LTfloat far;

    LTPerspective(LTfloat near, LTfloat origin, LTfloat far, LTSceneNode *child);
    
    virtual void draw();
    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTPitch : LTWrapNode {
    LTfloat pitch;

    LTPitch(LTfloat pitch, LTSceneNode *child);

    virtual void draw();
    virtual LTfloat* field_ptr(const char *field_name);
};

#endif
