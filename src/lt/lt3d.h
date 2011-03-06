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

struct LTPerspective : LTSceneNode {
    LTfloat near;
    LTfloat origin;
    LTfloat far;
    LTSceneNode *child;

    LTPerspective(LTfloat near, LTfloat origin, LTfloat far, LTSceneNode *child);
    
    virtual void draw();
    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTPitch : LTSceneNode {
    LTfloat pitch;
    LTSceneNode *child;

    LTPitch(LTfloat pitch, LTSceneNode *child);

    virtual void draw();
    virtual LTfloat* field_ptr(const char *field_name);
};

/*
struct LTTranslateZNode : LTSceneNode {
    LTfloat z;
    LTSceneNode *child;

    LTTranslateZNode(LTfloat z);
    virtual ~LTTranslateZNode();

    virtual LTfloat* field_ptr(const char *field_name);
    virtual void draw();
}
*/

#endif
