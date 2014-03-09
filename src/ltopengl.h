/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltopengl)

typedef GLuint          LTvertbuf;
typedef GLuint          LTframebuf;
typedef GLuint          LTtexid;
typedef GLushort        LTvertindex;
typedef GLshort         LTtexcoord;

enum LTDepthFunc {
    LT_DEPTH_FUNC_LEQUAL = GL_LEQUAL,
};

enum LTFogMode {
    LT_FOG_MODE_LINEAR = GL_LINEAR,
};

enum LTMatrixMode {
    LT_MATRIX_MODE_MODELVIEW = GL_MODELVIEW,
    LT_MATRIX_MODE_PROJECTION = GL_PROJECTION,
    LT_MATRIX_MODE_TEXTURE = GL_TEXTURE
};

enum LTVertDataType {
    LT_VERT_DATA_TYPE_FLOAT = GL_FLOAT,
    LT_VERT_DATA_TYPE_SHORT = GL_SHORT,
    LT_VERT_DATA_TYPE_BYTE  = GL_BYTE,
    LT_VERT_DATA_TYPE_UBYTE = GL_UNSIGNED_BYTE,
};

enum LTDrawMode {
    LT_DRAWMODE_TRIANGLES      = GL_TRIANGLES,
    LT_DRAWMODE_TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
    LT_DRAWMODE_TRIANGLE_FAN   = GL_TRIANGLE_FAN,
    LT_DRAWMODE_POINTS         = GL_POINTS,
    LT_DRAWMODE_LINES          = GL_LINES,
    LT_DRAWMODE_LINE_STRIP     = GL_LINE_STRIP,
    LT_DRAWMODE_LINE_LOOP      = GL_LINE_LOOP,
};

enum LTBlendMode {
    LT_BLEND_MODE_NORMAL,
    LT_BLEND_MODE_INVERT,
    LT_BLEND_MODE_ADD,
    LT_BLEND_MODE_SUBTRACT,
    //LT_BLEND_MODE_DIFF,
    LT_BLEND_MODE_COLOR,
    LT_BLEND_MODE_MULTIPLY,
    LT_BLEND_MODE_OFF,
};

enum LTTextureMode {
    LT_TEXTURE_MODE_MODULATE = GL_MODULATE,
    LT_TEXTURE_MODE_ADD = GL_ADD,
    LT_TEXTURE_MODE_DECAL = GL_DECAL,
    LT_TEXTURE_MODE_BLEND = GL_BLEND,
    LT_TEXTURE_MODE_REPLACE = GL_REPLACE,
};

enum LTTextureFilter {
    LT_TEXTURE_FILTER_LINEAR = GL_LINEAR,
    LT_TEXTURE_FILTER_NEAREST = GL_NEAREST,
};

enum LTCullMode {
    LT_CULL_BACK,
    LT_CULL_FRONT,
    LT_CULL_OFF,
};

void ltInitGLState();

void ltEnableTexturing();
void ltDisableTexturing();
void ltEnableTextureCoordArrays();
void ltDisableTextureCoordArrays();
void ltTextureMode(LTTextureMode mode);
void ltTextureMagFilter(LTTextureFilter filter);
void ltTextureMinFilter(LTTextureFilter filter);
void ltBindTexture(LTtexid texture_id);
LTtexid ltGenTexture();
void ltDeleteTexture(LTtexid);
void ltTexImage(int width, int height, void *data);

void ltBlendMode(LTBlendMode mode);

void ltEnableDepthTest();
void ltDisableDepthTest();
void ltEnableDepthMask();
void ltDisableDepthMask();
void ltDepthFunc(LTDepthFunc f);

void ltEnableDither();
void ltDisableDither();

void ltEnableAlphaTest();
void ltDisableAlphaTest();

void ltEnableStencilTest();
void ltDisableStencilTest();

void ltEnableVertexArrays();
void ltDisableVertexArrays();

void ltEnableIndexArrays();
void ltDisableIndexArrays();

void ltEnableColorArrays();
void ltDisableColorArrays();

void ltEnableNormalArrays();
void ltDisableNormalArrays();

void ltEnableFog();
void ltDisableFog();
void ltFogColor(LTfloat r, LTfloat g, LTfloat b);
void ltFogStart(LTfloat start);
void ltFogEnd(LTfloat end);
void ltFogMode(LTFogMode mode);

void ltBindFramebuffer(LTframebuf fb);

void ltClearColor(LTfloat r, LTfloat g, LTfloat b, LTfloat a);
void ltClear(bool color, bool depthbuf);

void ltColor(LTfloat r, LTfloat g, LTfloat b, LTfloat a);

void ltEnableLighting();
void ltDisableLighting();

void ltEnableLight(int light);
void ltDisableLight(int light);
void ltLightAmbient(int light, LTfloat r, LTfloat g, LTfloat b);
void ltLightDiffuse(int light, LTfloat r, LTfloat g, LTfloat b);
void ltLightSpecular(int light, LTfloat r, LTfloat g, LTfloat b);
void ltLightPosition(int light, LTfloat x, LTfloat y, LTfloat z, LTfloat w);
void ltLightAttenuation(int light, LTfloat q, LTfloat l, LTfloat c);

void ltMaterialShininess(LTfloat shininess);
void ltMaterialAmbient(LTfloat r, LTfloat g, LTfloat b);
void ltMaterialDiffuse(LTfloat r, LTfloat g, LTfloat b, LTfloat a);
void ltMaterialSpecular(LTfloat r, LTfloat g, LTfloat b);
void ltMaterialEmission(LTfloat r, LTfloat g, LTfloat b);

void ltCullFace(LTCullMode);

void ltMatrixMode(LTMatrixMode mode);
void ltPushMatrix();
void ltPopMatrix();
void ltLoadIdentity();
void ltMultMatrix(LTfloat *mat);
void ltOrtho(LTfloat left, LTfloat right, LTfloat bottom, LTfloat top, LTfloat nearz, LTfloat farz);
void ltFrustum(LTfloat left, LTfloat right, LTfloat bottom, LTfloat top, LTfloat nearz, LTfloat farz);

void ltRotate(LTfloat angle, LTfloat x, LTfloat y, LTfloat z);
void ltScale(LTfloat x, LTfloat y, LTfloat z);
void ltTranslate(LTfloat x, LTfloat y, LTfloat z);

void ltViewport(int x, int y, int width, int height);

LTvertbuf ltGenVertBuffer();
void ltDeleteVertBuffer(LTvertbuf vb);
void ltBindVertBuffer(LTvertbuf vb);
void ltStaticVertBufferData(int size, const void *data);
void ltVertexPointer(int size, LTVertDataType type, int stride, void *data);
void ltColorPointer(int size, LTVertDataType type, int stride, void *data);
void ltNormalPointer(LTVertDataType type, int stride, void *data);
void ltTexCoordPointer(int size, LTVertDataType type, int stride, void *data);
void ltDrawArrays(LTDrawMode mode, int start, int count);
void ltDrawElements(LTDrawMode mode, int n, LTvertindex *indices);

LTframebuf ltGenFramebuffer();
void ltDeleteFramebuffer(LTframebuf fb);
void ltFramebufferTexture(LTtexid texture_id);
bool ltFramebufferComplete();
