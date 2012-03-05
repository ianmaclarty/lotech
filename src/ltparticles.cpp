/* Copyright (C) 2010 Ian MacLarty */

#include "ltparticles.h"
#include "ltutil.h"

#include "LTParticleSystem_fields.h"

ct_assert(sizeof(LTPoint) == 8);
ct_assert(sizeof(LTCompactColor) == 4);

LTParticleSystem::LTParticleSystem(LTImage *img, int n)
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
    
    aspect_ratio = img->bb_width / img->bb_height;

    max_particles = n;
    emission_rate = -1.0f;

    num_particles = 0;
    emit_counter = 0.0f;

    particles = new LTParticle[n];
    // We maintain a reference to the LTImage object in the Lua wrapper.
    texture_id = img->atlas->texture_id;
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
    indices = new GLushort[n * 6];
    for (int i = 0; i < n; i++) {
        int i6 = i * 6;
        int i4 = i * 4;
        indices[i6+0] = (GLushort) i4+0;
        indices[i6+1] = (GLushort) i4+1;
        indices[i6+2] = (GLushort) i4+2;
        indices[i6+5] = (GLushort) i4+1;
        indices[i6+4] = (GLushort) i4+2;
        indices[i6+3] = (GLushort) i4+3;
    }

    fixture = NULL;

    glGenBuffers(1, &quads_vbo);
    glGenBuffers(1, &indices_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quads_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quads[0]) * n, quads, GL_STATIC_DRAW); // try GL_DYNAMIC_DRAW
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * n * 6, indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

LTParticleSystem::~LTParticleSystem() {
    glDeleteBuffers(1, &quads_vbo);
    glDeleteBuffers(1, &indices_vbo);
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

static GLfloat clamp(GLfloat f) {
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

            GLfloat size_2_x = (p->size / 2.0f) * aspect_ratio;
            GLfloat size_2_y = (p->size / 2.0f);
            if( p->rotation ) {
                GLfloat x1 = -size_2_x;
                GLfloat y1 = -size_2_y;

                GLfloat x2 = size_2_x;
                GLfloat y2 = size_2_y;
                GLfloat x = p->pos.x;
                GLfloat y = p->pos.y;

                GLfloat r = (GLfloat) -(p->rotation * LT_RADIANS_PER_DEGREE);
                GLfloat cr = cosf(r);
                GLfloat sr = sinf(r);
                GLfloat ax = x1 * cr - y1 * sr + x;
                GLfloat ay = x1 * sr + y1 * cr + y;
                GLfloat bx = x2 * cr - y1 * sr + x;
                GLfloat by = x2 * sr + y1 * cr + y;
                GLfloat cx = x2 * cr - y2 * sr + x;
                GLfloat cy = x2 * sr + y2 * cr + y;
                GLfloat dx = x1 * cr - y2 * sr + x;
                GLfloat dy = x1 * sr + y2 * cr + y;

                quad->bottom_left.vertex.x = ax;
                quad->bottom_left.vertex.y = ay;

                quad->bottom_right.vertex.x = bx;
                quad->bottom_right.vertex.y = by;

                quad->top_left.vertex.x = dx;
                quad->top_left.vertex.y = dy;

                quad->top_right.vertex.x = cx;
                quad->top_right.vertex.y = cy;
            } else {
                quad->bottom_left.vertex.x = p->pos.x - size_2_x;
                quad->bottom_left.vertex.y = p->pos.y - size_2_y;

                quad->bottom_right.vertex.x = p->pos.x + size_2_x;
                quad->bottom_right.vertex.y = p->pos.y - size_2_y;

                quad->top_left.vertex.x = p->pos.x - size_2_x;
                quad->top_left.vertex.y = p->pos.y + size_2_y;

                quad->top_right.vertex.x = p->pos.x + size_2_x;
                quad->top_right.vertex.y = p->pos.y + size_2_y;
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

    glBindBuffer(GL_ARRAY_BUFFER, quads_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quads[0]) * num_particles, quads);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void LTParticleSystem::draw() {
    if (num_particles > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, quads_vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);

        glEnableClientState(GL_COLOR_ARRAY);
        ltEnableTexture(texture_id);

        GLsizei stride = sizeof(LTParticleVertexData);
        glVertexPointer(2, GL_FLOAT, stride, 0);
        glColorPointer(4, GL_UNSIGNED_BYTE, stride, (GLvoid*)offsetof(LTParticleVertexData, color));
        glTexCoordPointer(2, GL_SHORT, stride, (GLvoid*)offsetof(LTParticleVertexData, tex_coord_x));
        
        glDrawElements(GL_TRIANGLES, num_particles * 6, GL_UNSIGNED_SHORT, 0);

        glDisableClientState(GL_COLOR_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        ltRestoreTint();
    }
}

LTfloat* LTParticleSystem::field_ptr(const char *field_name) {
    LTFieldInfo *field = LTParticleSystem_field_info(field_name, strlen(field_name));
    if (field != NULL) {
        LTfloat *ptr = (LTfloat*) ((char*)this + field->offset);
        return ptr;
    }
    return NULL;
}
