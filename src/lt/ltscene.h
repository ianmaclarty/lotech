/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTSCENE_H
#define LTSCENE_H

#include <list>

#ifdef LINUX
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#else
#include <OpenGL/GL.h>
#endif

#include "lttween.h"

enum LTSceneNodeStatus {
    LT_STATUS_VISIBLE,
    LT_STATUS_HIDDEN,
    LT_STATUS_DEAD            /* Delete from scene before next render. */
};

struct LTSceneNode : public LTTweenable {
    LTSceneNodeStatus   status;

    LTfloat      red;
    LTfloat      green;
    LTfloat      blue;
    LTfloat      alpha;

    LTfloat      x;
    LTfloat      y;
    LTdegrees    angle;
    LTfloat      x_scale;
    LTfloat      y_scale;

    std::list<LTSceneNode*> children;

    LTSceneNode();
    virtual ~LTSceneNode() {};

    virtual LTfloat* getFloatField(LTTweenFloatField f);

    virtual void preRenderChildren() {};
    virtual void postRenderChildren() {};
};

/*
struct LTBufferedImage : public LTSceneNode {
    GLuint  vertex_buffer_id;
    GLuint  texture_buffer_id;
    GLuint  texture_id;

    virtual void draw();
};
*/

struct LTUnitSquare : public LTSceneNode {
    virtual void preRenderChildren();
};

void ltRenderScene(LTSceneNode *scene);

#endif
