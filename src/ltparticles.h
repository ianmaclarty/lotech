/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltparticles)

// This is based on the Cocos2D particle system.

struct LTParticle {
    LTVec2 pos;
    LTColor color;
    LTColor delta_color;
    LTfloat size;
    LTfloat delta_size;
    LTdegrees rotation;
    LTdegrees delta_rotation;
    LTfloat time_to_live;
    LTVec2 dir;
    LTfloat radial_accel;
    LTfloat tangential_accel;
    LTfloat damping;
};

struct LTParticleVertexData {
    LTVec2 vertex;
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
    bool particles_active;
    LTfloat duration;
    LTfloat elapsed;
    LTVec2 source_position;
    LTVec2 source_position_variance;
    LTbool use_end_position;
    LTVec2 end_position;
    LTVec2 end_position_variance;
    LTdegrees angle;
    LTdegrees angle_variance;
    LTVec2 gravity;
    LTfloat speed;
    LTfloat speed_variance;
    LTfloat tangential_accel;
    LTfloat tangential_accel_variance;
    LTfloat radial_accel;
    LTfloat radial_accel_variance;
    LTfloat damping;
    LTfloat damping_variance;
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
    int max_particles_init;
    LTTexturedNode *img;

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

    LTFixture *fixture; // A fixture used to restrict start positions.

    LTParticleSystem();
    virtual ~LTParticleSystem();
    virtual void init(lua_State *L);

    bool is_full();
    void stop();
    void reset();
    void add_particle();
    virtual void advance(LTfloat dt);
    virtual void draw();
};

LTParticleSystem *lt_expect_LTParticleSystem(lua_State *L, int arg);
