/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTPARTICLES_H
#define LTPARTICLES_H

#include "ltcommon.h"
#include "ltgraphics.h"
#include "ltimage.h"
#include "ltphysics.h"
#include "ltscene.h"

// This is based on the Cocos2D particle system.

struct LTParticle {
    LTPoint pos;
    LTColor color;
    LTColor delta_color;
    LTfloat size;
    LTfloat delta_size;
    LTdegrees rotation;
    LTdegrees delta_rotation;
    LTfloat time_to_live;
    LTPoint dir;
    LTfloat radial_accel;
    LTfloat tangential_accel;
};

struct LTParticleVertexData {
    LTPoint vertex;
    LTCompactColor color;
    LTtexcoord tex_coord_x;
    LTtexcoord tex_coord_y;
};

struct LTParticleQuad {
    LTParticleVertexData bottom_left;
    LTParticleVertexData bottom_right;
    LTParticleVertexData top_left;
    LTParticleVertexData top_right;
};

struct LTParticleSystem : LTSceneNode {
    bool active;
    LTfloat duration;
    LTfloat elapsed;
    LTPoint source_position;
    LTPoint source_position_variance;
    LTdegrees angle;
    LTdegrees angle_variance;
    LTPoint gravity;
    LTfloat speed;
    LTfloat speed_variance;
    LTfloat tangential_accel;
    LTfloat tangential_accel_variance;
    LTfloat radial_accel;
    LTfloat radial_accel_variance;
    LTfloat start_size;
    LTfloat start_size_variance;
    LTfloat end_size;
    LTfloat end_size_variance;
    LTfloat life;
    LTfloat life_variance;
    LTColor start_color;
    LTColor start_color_variance;
    LTColor end_color;
    LTColor end_color_variance;
    LTfloat start_spin;
    LTfloat start_spin_variance;
    LTfloat end_spin;
    LTfloat end_spin_variance;
    LTfloat emission_rate;

    int max_particles;
    int num_particles;
    LTfloat emit_counter;
    LTtexid texture_id;
    LTParticle *particles;
    LTParticleQuad *quads;
    LTushort *indices;
    LTfloat img_left;
    LTfloat img_right;
    LTfloat img_bottom;
    LTfloat img_top;

    LTFixture *fixture; // A fixture to restrict start positions.

    LTParticleSystem(LTTexturedNode *img, int max_particles);
    virtual ~LTParticleSystem();

    bool is_full();
    void stop();
    void reset();
    void add_particle();
    void advance(LTfloat dt);
    virtual void draw();

    virtual LTfloat* field_ptr(const char *field_name);
};

#endif
