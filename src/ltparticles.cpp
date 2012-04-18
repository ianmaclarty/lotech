/* Copyright (C) 2010 Ian MacLarty */

#include "lt.h"

ct_assert(sizeof(LTPoint) == 8);
ct_assert(sizeof(LTCompactColor) == 4);

LTParticleSystem::LTParticleSystem(LTTexturedNode *img, int n)
        : LTSceneNode(LT_TYPE_PARTICLESYSTEM) {
    active = true;
    duration = -1.0f;
    elapsed = 0.0f;
    angle = 0.0f;
    angle_variance = 0.0f;
    speed = 1.0f;
    speed_variance = 0.0f;
    tangential_accel = 0.0f;
    tangential_accel_variance = 0.0f;
    radial_accel = 0.0f;
    radial_accel_variance = 0.0f;
    start_size = 1.0f;
    start_size_variance = 0.0f;
    end_size = 1.0f;
    end_size_variance = 0.0f;
    life = 1.0f;
    life_variance = 0.0f;
    start_color_variance.r = 0.0f;
    start_color_variance.g = 0.0f;
    start_color_variance.b = 0.0f;
    start_color_variance.a = 0.0f;
    end_color_variance.r = 0.0f;
    end_color_variance.g = 0.0f;
    end_color_variance.b = 0.0f;
    end_color_variance.a = 0.0f;
    start_spin = 0.0f;
    start_spin_variance = 0.0f;
    end_spin = 0.0f;
    end_spin_variance = 0.0f;
    
    max_particles = n;
    emission_rate = -1.0f;

    num_particles = 0;
    emit_counter = 0.0f;

    particles = new LTParticle[n];
    // We maintain a reference to the LTTexturedNode object in the Lua wrapper.
    texture_id = img->texture_id;
    img_left = img->world_vertices[0];
    img_right = img->world_vertices[2];
    img_bottom = img->world_vertices[5];
    img_top = img->world_vertices[1];

    quads = new LTParticleQuad[n];

    for (int i = 0; i < n; i++) {
        quads[i].bottom_left.tex_coord_x  = img->tex_coords[6];
        quads[i].bottom_left.tex_coord_y  = img->tex_coords[7];
        quads[i].bottom_right.tex_coord_x = img->tex_coords[4];
        quads[i].bottom_right.tex_coord_y = img->tex_coords[5];
        quads[i].top_left.tex_coord_x     = img->tex_coords[0];
        quads[i].top_left.tex_coord_y     = img->tex_coords[1];
        quads[i].top_right.tex_coord_x    = img->tex_coords[2];
        quads[i].top_right.tex_coord_y    = img->tex_coords[3];
    }
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

    fixture = NULL;
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
    active = false;
    elapsed = duration;
    emit_counter = 0.0f;
}

void LTParticleSystem::reset() {
    active = true;
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
                active = false;
                return;
            }
            LTBody *b = fixture->body;
            if (b == NULL) {
                // Fixture not attached to any bodies.
                active = false;
                return;
            }
            LTWorld *w = b->world;
            if (w == NULL) {
                // Shouldn't happen, but just in case.
                active = false;
                return;
            }
            LTfloat s = w->scaling;
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
        start.r = clamp(start_color.r + start_color_variance.r * ltRandMinus1_1());
        start.g = clamp(start_color.g + start_color_variance.g * ltRandMinus1_1());
        start.b = clamp(start_color.b + start_color_variance.b * ltRandMinus1_1());
        start.a = clamp(start_color.a + start_color_variance.a * ltRandMinus1_1());

        LTColor end;
        end.r = clamp(end_color.r + end_color_variance.r * ltRandMinus1_1());
        end.g = clamp(end_color.g + end_color_variance.g * ltRandMinus1_1());
        end.b = clamp(end_color.b + end_color_variance.b * ltRandMinus1_1());
        end.a = clamp(end_color.a + end_color_variance.a * ltRandMinus1_1());

        p->color = start;
        p->delta_color.r = (end.r - start.r) / p->time_to_live;
        p->delta_color.g = (end.g - start.g) / p->time_to_live;
        p->delta_color.b = (end.b - start.b) / p->time_to_live;
        p->delta_color.a = (end.a - start.a) / p->time_to_live;

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

        LTfloat a = LT_RADIANS_PER_DEGREE * (angle + angle_variance * ltRandMinus1_1());
        LTfloat v_x = cosf(a);
        LTfloat v_y = sinf(a);
        LTfloat s = speed + speed_variance * ltRandMinus1_1();

        p->dir.x = v_x * s;
        p->dir.y = v_y * s;

        p->radial_accel = radial_accel + radial_accel_variance * ltRandMinus1_1();
        p->tangential_accel = tangential_accel + tangential_accel_variance * ltRandMinus1_1();

        num_particles++;
    }
}

void LTParticleSystem::advance(LTfloat dt) {
    if (!executeActions(dt)) return;

    if (active && emission_rate > 0.0f) {
        LTfloat rate = 1.0f / emission_rate;
        emit_counter += dt;
        while (num_particles < max_particles && emit_counter > rate && active) {
            add_particle();
            emit_counter -= rate;
        }

        elapsed += dt;
        if ((duration != -1.0f && duration < elapsed) || !active) {
            stop();
        }
    }

    int i = 0;

    while (i < num_particles) {
        LTParticle *p = &particles[i];
        p->time_to_live -= dt;
        if (p->time_to_live > 0.0f) {
            LTPoint tmp, radial, tangential;
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
            p->pos.x += p->dir.x * dt;
            p->pos.y += p->dir.y * dt;
            p->color.r += p->delta_color.r * dt;
            p->color.g += p->delta_color.g * dt;
            p->color.b += p->delta_color.b * dt;
            p->color.a += p->delta_color.a * dt;
            p->size += p->delta_size * dt;
            if (p->size < 0.0f) {
                p->size = 0.0f;
            }
            p->rotation += p->delta_rotation * dt;
            
            LTParticleQuad *quad = &quads[i];
            LTCompactColor color(
                p->color.r * 255,
                p->color.g * 255,
                p->color.b * 255,
                p->color.a * 255
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

LTFieldDescriptor* LTParticleSystem::fields() {
    static LTFieldDescriptor flds[] = {
        {"duration", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(duration), NULL, NULL, LT_ACCESS_FULL},
        {"elapsed", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(elapsed), NULL, NULL, LT_ACCESS_FULL},
        {"life", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(life), NULL, NULL, LT_ACCESS_FULL},
        {"life_variance", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(life_variance), NULL, NULL, LT_ACCESS_FULL},
        {"emission_rate", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(emission_rate), NULL, NULL, LT_ACCESS_FULL},
        {"source_position_x", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(source_position.x), NULL, NULL, LT_ACCESS_FULL},
        {"source_position_y", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(source_position.y), NULL, NULL, LT_ACCESS_FULL},
        {"source_position_variance_x", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(source_position_variance.x), NULL, NULL, LT_ACCESS_FULL},
        {"source_position_variance_y", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(source_position_variance.y), NULL, NULL, LT_ACCESS_FULL},
        {"angle", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(angle), NULL, NULL, LT_ACCESS_FULL},
        {"angle_variance", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(angle_variance), NULL, NULL, LT_ACCESS_FULL},
        {"gravity_x", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(gravity.x), NULL, NULL, LT_ACCESS_FULL},
        {"gravity_y", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(gravity.y), NULL, NULL, LT_ACCESS_FULL},
        {"speed", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(speed), NULL, NULL, LT_ACCESS_FULL},
        {"speed_variance", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(speed_variance), NULL, NULL, LT_ACCESS_FULL},
        {"tangential_accel", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(tangential_accel), NULL, NULL, LT_ACCESS_FULL},
        {"tangential_accel_variance", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(tangential_accel_variance), NULL, NULL, LT_ACCESS_FULL},
        {"radial_accel", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(radial_accel), NULL, NULL, LT_ACCESS_FULL},
        {"radial_accel_variance", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(radial_accel_variance), NULL, NULL, LT_ACCESS_FULL},
        {"start_size", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(start_size), NULL, NULL, LT_ACCESS_FULL},
        {"start_size_variance", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(start_size_variance), NULL, NULL, LT_ACCESS_FULL},
        {"end_size", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(end_size), NULL, NULL, LT_ACCESS_FULL},
        {"end_size_variance", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(end_size_variance), NULL, NULL, LT_ACCESS_FULL},
        {"start_color_red", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(start_color.r), NULL, NULL, LT_ACCESS_FULL},
        {"start_color_green", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(start_color.g), NULL, NULL, LT_ACCESS_FULL},
        {"start_color_blue", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(start_color.b), NULL, NULL, LT_ACCESS_FULL},
        {"start_color_alpha", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(start_color.a), NULL, NULL, LT_ACCESS_FULL},
        {"start_color_variance_red", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(start_color_variance.r), NULL, NULL, LT_ACCESS_FULL},
        {"start_color_variance_green", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(start_color_variance.g), NULL, NULL, LT_ACCESS_FULL},
        {"start_color_variance_blue", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(start_color_variance.b), NULL, NULL, LT_ACCESS_FULL},
        {"start_color_variance_alpha", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(start_color_variance.a), NULL, NULL, LT_ACCESS_FULL},
        {"end_color_red", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(end_color.r), NULL, NULL, LT_ACCESS_FULL},
        {"end_color_green", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(end_color.g), NULL, NULL, LT_ACCESS_FULL},
        {"end_color_blue", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(end_color.b), NULL, NULL, LT_ACCESS_FULL},
        {"end_color_alpha", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(end_color.a), NULL, NULL, LT_ACCESS_FULL},
        {"end_color_variance_red", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(end_color_variance.r), NULL, NULL, LT_ACCESS_FULL},
        {"end_color_variance_green", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(end_color_variance.g), NULL, NULL, LT_ACCESS_FULL},
        {"end_color_variance_blue", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(end_color_variance.b), NULL, NULL, LT_ACCESS_FULL},
        {"end_color_variance_alpha", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(end_color_variance.a), NULL, NULL, LT_ACCESS_FULL},
        {"start_spin", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(start_spin), NULL, NULL, LT_ACCESS_FULL},
        {"start_spin_variance", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(start_spin_variance), NULL, NULL, LT_ACCESS_FULL},
        {"end_spin", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(end_spin), NULL, NULL, LT_ACCESS_FULL},
        {"end_spin_variance", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(end_spin_variance), NULL, NULL, LT_ACCESS_FULL},
        LT_END_FIELD_DESCRIPTOR_LIST,
    };
    return flds;
}
