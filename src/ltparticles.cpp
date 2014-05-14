/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
/* Some of this code was adapted from Cocos2D */
#include "lt.h"

LT_INIT_IMPL(ltparticles)

ct_assert(sizeof(LTVec2) == 8);
ct_assert(sizeof(LTCompactColor) == 4);

struct LTParticleSystemAction : LTAction {
    LTParticleSystemAction(LTSceneNode *node) : LTAction(node) {};
    virtual bool doAction(LTfloat dt) {
        LTParticleSystem *particles = (LTParticleSystem*)node;
        particles->advance(dt);
        return false;
    }
};

LTParticleSystem::LTParticleSystem() {
    particles_active = true;
    duration = -1.0f;
    speed = 1.0f;
    start_size = 1.0f;
    end_size = 1.0f;
    life = 1.0f;
    start_color_variance.red = 0.0f;
    start_color_variance.green = 0.0f;
    start_color_variance.blue = 0.0f;
    start_color_variance.alpha = 0.0f;
    end_color_variance.red = 0.0f;
    end_color_variance.green = 0.0f;
    end_color_variance.blue = 0.0f;
    end_color_variance.alpha = 0.0f;
    emission_rate = -1.0f;
    fixture = NULL;
    add_action(new LTParticleSystemAction(this));
}

static void setup_img(LTParticleSystem *p) {
    p->texture_id = p->img->texture_id;
    p->img_left = p->img->world_vertices[0];
    p->img_right = p->img->world_vertices[2];
    p->img_bottom = p->img->world_vertices[1];
    p->img_top = p->img->world_vertices[5];

    if (p->quads != NULL) {
        delete[] p->quads;
    }

    int n = p->max_particles;

    p->quads = new LTParticleQuad[n];

    for (int i = 0; i < n; i++) {
        p->quads[i].bottom_left.tex_coord_x  = p->img->tex_coords[0];
        p->quads[i].bottom_left.tex_coord_y  = p->img->tex_coords[1];
        p->quads[i].bottom_right.tex_coord_x = p->img->tex_coords[2];
        p->quads[i].bottom_right.tex_coord_y = p->img->tex_coords[3];
        p->quads[i].top_right.tex_coord_x    = p->img->tex_coords[4];
        p->quads[i].top_right.tex_coord_y    = p->img->tex_coords[5];
        p->quads[i].top_left.tex_coord_x     = p->img->tex_coords[6];
        p->quads[i].top_left.tex_coord_y     = p->img->tex_coords[7];
    }
}

void LTParticleSystem::init(lua_State *L) {
    max_particles = max_particles_init;
    if (max_particles <= 0) {
        luaL_error(L, "max_particles must be set to a positive value");
    }
    if (img == NULL) {
        luaL_error(L, "img must be specified");
    }
    if (emission_rate < 0.0f) {
        emission_rate = (LTfloat)max_particles / life;
    }
    num_particles = 0;
    emit_counter = 0.0f;

    int n = max_particles;
    particles = new LTParticle[n];

    setup_img(this);

    indices = new LTushort[n * 6];
    for (int i = 0; i < n; i++) {
        int i6 = i * 6;
        int i4 = i * 4;
        indices[i6+0] = (LTushort) i4+0;
        indices[i6+1] = (LTushort) i4+1;
        indices[i6+2] = (LTushort) i4+2;
        indices[i6+5] = (LTushort) i4+1;
        indices[i6+4] = (LTushort) i4+2;
        indices[i6+3] = (LTushort) i4+3;
    }
}

LTParticleSystem::~LTParticleSystem() {
    delete[] particles;
    delete[] quads;
    delete[] indices;
}

bool LTParticleSystem::is_full() {
    return num_particles == max_particles;
}

void LTParticleSystem::stop() {
    particles_active = false;
    elapsed = duration;
    emit_counter = 0.0f;
}

void LTParticleSystem::reset() {
    particles_active = true;
    elapsed = 0.0f;
    for (int i = 0; i < num_particles; ++i) {
        particles[i].time_to_live = 0.0f;
    }
}

static LTfloat clamp(LTfloat f) {
    return f > 1.0f ? 1.0f : (f < 0.0f ? 0.0f : f);
}

void LTParticleSystem::add_particle() {
    if (!is_full()) {
        LTParticle *p = &particles[num_particles];
        
        p->time_to_live = life + life_variance * ltRandMinus1_1();
        if (p->time_to_live < 0.001f) {
            p->time_to_live = 0.001f; // Avoid division by zero.
        }

        if (fixture != NULL) {
            b2Fixture *f = fixture->fixture;
            if (f == NULL) {
                // Fixture has been deleted, so don't create any new particles.
                particles_active = false;
                return;
            }
            LTBody *b = fixture->body;
            if (b == NULL) {
                // Fixture not attached to any bodies.
                particles_active = false;
                return;
            }
            LTWorld *w = b->world;
            LTfloat s = w->scale;
            b2AABB aabb = f->GetAABB(0);
            int num_tries = 20;
            b2Vec2 test_point;
            do {
                test_point.x = ltRandBetween(aabb.lowerBound.x, aabb.upperBound.x);
                test_point.y = ltRandBetween(aabb.lowerBound.y, aabb.upperBound.y);
                num_tries--;
            } while (!f->TestPoint(test_point) && num_tries > 0);
            p->pos.x = test_point.x * s;
            p->pos.y = test_point.y * s;
        } else {
            p->pos.x = source_position.x + source_position_variance.x * ltRandMinus1_1();
            p->pos.y = source_position.y + source_position_variance.y * ltRandMinus1_1();
        }

        LTColor start;
        start.red = clamp(start_color.red + start_color_variance.red * ltRandMinus1_1());
        start.green = clamp(start_color.green + start_color_variance.green * ltRandMinus1_1());
        start.blue = clamp(start_color.blue + start_color_variance.blue * ltRandMinus1_1());
        start.alpha = clamp(start_color.alpha + start_color_variance.alpha * ltRandMinus1_1());

        LTColor end;
        end.red = clamp(end_color.red + end_color_variance.red * ltRandMinus1_1());
        end.green = clamp(end_color.green + end_color_variance.green * ltRandMinus1_1());
        end.blue = clamp(end_color.blue + end_color_variance.blue * ltRandMinus1_1());
        end.alpha = clamp(end_color.alpha + end_color_variance.alpha * ltRandMinus1_1());

        p->color = start;
        p->delta_color.red = (end.red - start.red) / p->time_to_live;
        p->delta_color.green = (end.green - start.green) / p->time_to_live;
        p->delta_color.blue = (end.blue - start.blue) / p->time_to_live;
        p->delta_color.alpha = (end.alpha - start.alpha) / p->time_to_live;

        p->size = start_size + start_size_variance * ltRandMinus1_1();
        if (p->size < 0.0f) {
            p->size = 0.0f;
        }
        LTfloat endS = end_size + end_size_variance * ltRandMinus1_1();
        if (endS < 0.0f) {
            endS = 0.0f;
        }
        p->delta_size = (endS - p->size) / p->time_to_live;
    
        LTfloat startA = start_spin + start_spin_variance * ltRandMinus1_1();
        LTfloat endA = end_spin + end_spin_variance * ltRandMinus1_1();
        p->rotation = startA;
        p->delta_rotation = (endA - startA) / p->time_to_live;

        LTfloat v_x;
        LTfloat v_y;
        if (use_end_position) {
            LTVec2 end_pos;
            end_pos.x = end_position.x + end_position_variance.x * ltRandMinus1_1();
            end_pos.y = end_position.y + end_position_variance.y * ltRandMinus1_1();
            LTfloat dx = end_pos.x - p->pos.x;
            LTfloat dy = end_pos.y - p->pos.y;
            LTfloat d = sqrtf(dx*dx + dy*dy);
            v_x = dx/p->time_to_live;
            v_y = dy/p->time_to_live;
            p->dir.x = v_x;
            p->dir.y = v_y;
        } else {
            LTfloat a;
            a = LT_RADIANS_PER_DEGREE * (angle + angle_variance * ltRandMinus1_1());
            v_x = cosf(a);
            v_y = sinf(a);
            LTfloat s = speed + speed_variance * ltRandMinus1_1();
            p->dir.x = v_x * s;
            p->dir.y = v_y * s;
        }

        p->radial_accel = radial_accel + radial_accel_variance * ltRandMinus1_1();
        p->tangential_accel = tangential_accel + tangential_accel_variance * ltRandMinus1_1();
        p->damping = damping + damping_variance * ltRandMinus1_1();

        num_particles++;
    }
}

void LTParticleSystem::advance(LTfloat dt) {
    //if (!executeActions(dt)) return;

    if (particles_active && emission_rate > 0.0f) {
        LTfloat rate = 1.0f / emission_rate;
        emit_counter += dt;
        while (num_particles < max_particles && emit_counter > rate && particles_active) {
            add_particle();
            emit_counter -= rate;
        }

        elapsed += dt;
        if ((duration != -1.0f && duration < elapsed) || !particles_active) {
            stop();
        }
    }

    int i = 0;

    while (i < num_particles) {
        LTParticle *p = &particles[i];
        p->time_to_live -= dt;
        if (p->time_to_live > 0.0f) {
            LTVec2 tmp, radial, tangential;
            radial = p->pos;
            radial.normalize();
            tangential = radial;
            radial.x *= p->radial_accel;
            radial.y *= p->radial_accel;
            LTfloat newy = tangential.x;
            tangential.x = -tangential.y;
            tangential.y = newy;
            tangential.x *= p->tangential_accel;
            tangential.y *= p->tangential_accel;
            p->dir.x += (radial.x + tangential.x + gravity.x) * dt;
            p->dir.y += (radial.y + tangential.y + gravity.y) * dt;
            p->dir.x *= 1.0f - dt * p->damping;
            p->dir.y *= 1.0f - dt * p->damping;
            p->pos.x += p->dir.x * dt;
            p->pos.y += p->dir.y * dt;
            p->color.red += p->delta_color.red * dt;
            p->color.green += p->delta_color.green * dt;
            p->color.blue += p->delta_color.blue * dt;
            p->color.alpha += p->delta_color.alpha * dt;
            p->size += p->delta_size * dt;
            if (p->size < 0.0f) {
                p->size = 0.0f;
            }
            p->rotation += p->delta_rotation * dt;
            
            LTParticleQuad *quad = &quads[i];
            LTCompactColor color(
                p->color.red * 255,
                p->color.green * 255,
                p->color.blue * 255,
                p->color.alpha * 255
            );
            quad->bottom_left.color = color;
            quad->bottom_right.color = color;
            quad->top_left.color = color;
            quad->top_right.color = color;

            LTfloat x1 = img_left * p->size;
            LTfloat y1 = img_bottom * p->size;
            LTfloat x2 = img_right * p->size;
            LTfloat y2 = img_top * p->size;

            if( p->rotation ) {

                LTfloat x = p->pos.x;
                LTfloat y = p->pos.y;

                LTfloat r = (LTfloat) -(p->rotation * LT_RADIANS_PER_DEGREE);
                LTfloat cr = cosf(r);
                LTfloat sr = sinf(r);
                LTfloat ax = x1 * cr - y1 * sr + x;
                LTfloat ay = x1 * sr + y1 * cr + y;
                LTfloat bx = x2 * cr - y1 * sr + x;
                LTfloat by = x2 * sr + y1 * cr + y;
                LTfloat cx = x2 * cr - y2 * sr + x;
                LTfloat cy = x2 * sr + y2 * cr + y;
                LTfloat dx = x1 * cr - y2 * sr + x;
                LTfloat dy = x1 * sr + y2 * cr + y;

                quad->bottom_left.vertex.x = ax;
                quad->bottom_left.vertex.y = ay;

                quad->bottom_right.vertex.x = bx;
                quad->bottom_right.vertex.y = by;

                quad->top_left.vertex.x = dx;
                quad->top_left.vertex.y = dy;

                quad->top_right.vertex.x = cx;
                quad->top_right.vertex.y = cy;
            } else {
                quad->bottom_left.vertex.x = p->pos.x + x1;
                quad->bottom_left.vertex.y = p->pos.y + y1;

                quad->bottom_right.vertex.x = p->pos.x + x2;
                quad->bottom_right.vertex.y = p->pos.y + y1;

                quad->top_left.vertex.x = p->pos.x + x1;
                quad->top_left.vertex.y = p->pos.y + y2;

                quad->top_right.vertex.x = p->pos.x + x2;
                quad->top_right.vertex.y = p->pos.y + y2;
            }
            i++;
        } else {
            // life < 0
            if (i != num_particles - 1) {
                particles[i] = particles[num_particles - 1];
            }
            num_particles--;
        }
    }
}

void LTParticleSystem::draw() {
    if (num_particles > 0) {
        ltBindVertBuffer(0);
        ltEnableColorArrays();
        ltEnableTexture(texture_id);

        int stride = sizeof(LTParticleVertexData);
        LTubyte *start = (unsigned char*)quads;
        LTubyte *color_start = (unsigned char*)&quads[0].bottom_left.color;
        LTubyte *tex_start = (unsigned char*)&quads[0].bottom_left.tex_coord_x;
        ltVertexPointer(2, LT_VERT_DATA_TYPE_FLOAT, stride, start);
        ltColorPointer(4, LT_VERT_DATA_TYPE_UBYTE, stride, color_start);
        ltTexCoordPointer(2, LT_VERT_DATA_TYPE_SHORT, stride, tex_start);
        
        ltDrawElements(LT_DRAWMODE_TRIANGLES, num_particles * 6, indices);

        ltDisableColorArrays();
        ltRestoreTint();
    }
}

static int particles_advance(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTParticleSystem *particles = lt_expect_LTParticleSystem(L, 1);
    LTfloat dt = luaL_checknumber(L, 2);
    particles->advance(dt);
    return 0;
}

static LTbool get_is_finished(LTObject *obj) {
    LTParticleSystem *p = (LTParticleSystem*)obj;
    return !(p->particles_active || p->num_particles > 0);
}

static LTObject* get_img(LTObject *o) {
    return ((LTParticleSystem*)o)->img;
}

static void set_img(LTObject *o, LTObject *v) {
    LTParticleSystem* p = (LTParticleSystem*)o;
    LTTexturedNode *img = (LTTexturedNode*)v;
    if (p->img == NULL) {
        // First time, setup_img will be called in init.
        p->img = img;
    } else {
        p->img = img;
        setup_img(p);
    }
}

LT_REGISTER_TYPE(LTParticleSystem, "lt.ParticleSystem", "lt.SceneNode")
LT_REGISTER_METHOD(LTParticleSystem, Advance, particles_advance)
LT_REGISTER_PROPERTY_OBJ(LTParticleSystem, img, LTTexturedNode, get_img, set_img);
LT_REGISTER_FIELD_INT_AS(LTParticleSystem, max_particles_init, "max_particles")
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, duration)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, life)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, life_variance)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, emission_rate)
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, source_position.x, "source_position_x")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, source_position.y, "source_position_y")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, source_position_variance.x, "source_position_variance_x")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, source_position_variance.y, "source_position_variance_y")
LT_REGISTER_FIELD_BOOL(LTParticleSystem, use_end_position)
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, end_position.x, "end_position_x")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, end_position.y, "end_position_y")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, end_position_variance.x, "end_position_variance_x")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, end_position_variance.y, "end_position_variance_y")
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, angle)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, angle_variance)
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, gravity.x, "gravity_x")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, gravity.y, "gravity_y")
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, speed)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, speed_variance)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, tangential_accel)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, tangential_accel_variance)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, radial_accel)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, radial_accel_variance)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, damping)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, damping_variance)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, start_size)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, start_size_variance)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, end_size)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, end_size_variance)
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, start_color.red, "start_color_red")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, start_color.green, "start_color_green")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, start_color.blue, "start_color_blue")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, start_color.alpha, "start_color_alpha")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, start_color_variance.red, "start_color_variance_red")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, start_color_variance.green, "start_color_variance_green")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, start_color_variance.blue, "start_color_variance_blue")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, start_color_variance.alpha, "start_color_variance_alpha")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, end_color.red, "end_color_red")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, end_color.green, "end_color_green")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, end_color.blue, "end_color_blue")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, end_color.alpha, "end_color_alpha")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, end_color_variance.red, "end_color_variance_red")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, end_color_variance.green, "end_color_variance_green")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, end_color_variance.blue, "end_color_variance_blue")
LT_REGISTER_FIELD_FLOAT_AS(LTParticleSystem, end_color_variance.alpha, "end_color_variance_alpha")
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, start_spin)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, start_spin_variance)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, end_spin)
LT_REGISTER_FIELD_FLOAT(LTParticleSystem, end_spin_variance)
LT_REGISTER_FIELD_OBJ(LTParticleSystem, fixture, LTFixture)
LT_REGISTER_PROPERTY_BOOL(LTParticleSystem, finished, get_is_finished, NULL)
