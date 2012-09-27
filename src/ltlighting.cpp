#include "lt.h"

LT_INIT_IMPL(ltlighting)

static int next_light_num = 0;

LTLightingNode::LTLightingNode() {
    enabled = true;
}

void LTLightingNode::draw() {
    static bool lighting_on = false;
    bool prev_lighting = lighting_on;
    if (enabled) {
        ltEnableLighting();
        lighting_on = true;
    } else {
        ltDisableLighting();
        lighting_on = false;
    }
    child->draw();
    if (prev_lighting) {
        ltEnableLighting();
    } else {
        ltDisableLighting();
    }
    lighting_on = prev_lighting;
}

LT_REGISTER_TYPE(LTLightingNode, "lt.Lighting", "lt.Wrap")
LT_REGISTER_FIELD_BOOL(LTLightingNode, enabled)

LTLight::LTLight() {
    ambient.red = 0;
    ambient.green = 0;
    ambient.blue = 0;
    diffuse.red = 1;
    diffuse.green = 1;
    diffuse.blue = 1;
    specular.red = 1;
    specular.green = 1;
    specular.blue = 1;
    position.x = 0;
    position.y = 0;
    position.z = 1;
    atten_c = 1;
    atten_l = 0;
    atten_q = 0;
    fixed = false;
}

void LTLight::draw() {
    int light_num = next_light_num;
    next_light_num++;
    ltEnableLight(light_num);
    ltLightAmbient(light_num, ambient.red, ambient.green, ambient.blue);
    ltLightDiffuse(light_num, diffuse.red, diffuse.green, diffuse.blue);
    ltLightSpecular(light_num, specular.red, specular.green, specular.blue);
    ltLightPosition(light_num, position.x, position.y, position.z, fixed ? 0 : 1);
    ltLightAttenuation(light_num, atten_q, atten_l, atten_c);
    child->draw();
    ltDisableLight(light_num);
    next_light_num--;
}

LT_REGISTER_TYPE(LTLight, "lt.Light", "lt.Wrap")
LT_REGISTER_FIELD_FLOAT_AS(LTLight, ambient.red, "ambient_red")
LT_REGISTER_FIELD_FLOAT_AS(LTLight, ambient.green, "ambient_green")
LT_REGISTER_FIELD_FLOAT_AS(LTLight, ambient.blue, "ambient_blue")
LT_REGISTER_FIELD_FLOAT_AS(LTLight, diffuse.red, "diffuse_red")
LT_REGISTER_FIELD_FLOAT_AS(LTLight, diffuse.green, "diffuse_green")
LT_REGISTER_FIELD_FLOAT_AS(LTLight, diffuse.blue, "diffuse_blue")
LT_REGISTER_FIELD_FLOAT_AS(LTLight, specular.red, "specular_red")
LT_REGISTER_FIELD_FLOAT_AS(LTLight, specular.green, "specular_green")
LT_REGISTER_FIELD_FLOAT_AS(LTLight, specular.blue, "specular_blue")
LT_REGISTER_FIELD_FLOAT_AS(LTLight, position.x, "x")
LT_REGISTER_FIELD_FLOAT_AS(LTLight, position.y, "y")
LT_REGISTER_FIELD_FLOAT_AS(LTLight, position.z, "z")
LT_REGISTER_FIELD_FLOAT_AS(LTLight, atten_c, "c")
LT_REGISTER_FIELD_FLOAT_AS(LTLight, atten_l, "l")
LT_REGISTER_FIELD_FLOAT_AS(LTLight, atten_q, "q")
LT_REGISTER_FIELD_BOOL(LTLight, fixed)

LTMaterial::LTMaterial() {
    ambient.red = 0.2f;
    ambient.green = 0.2f;
    ambient.blue = 0.2f;
    diffuse.red = 0.8f;
    diffuse.green = 0.8f;
    diffuse.blue = 0.8f;
    specular.red = 0;
    specular.green = 0;
    specular.blue = 0;
    emission.red = 0;
    emission.green = 0;
    emission.blue = 0;
    shininess = 0;
};

static std::vector<LTMaterial*> material_stack;

void LTMaterial::setup() {
    ltMaterialAmbient(ambient.red, ambient.green, ambient.blue);
    ltMaterialDiffuse(diffuse.red, diffuse.green, diffuse.blue, diffuse.alpha);
    ltMaterialSpecular(specular.red, specular.green, specular.blue);
    ltMaterialEmission(emission.red, emission.green, emission.blue);
    ltMaterialShininess(shininess);
}

void LTMaterial::draw() {
    material_stack.push_back(this);
    setup();
    child->draw();
    material_stack.pop_back();
    if (!material_stack.empty()) {
        material_stack.back()->setup();
    }
}

LT_REGISTER_TYPE(LTMaterial, "lt.Material", "lt.Wrap")
LT_REGISTER_FIELD_FLOAT_AS(LTMaterial, ambient.red, "ambient_red")
LT_REGISTER_FIELD_FLOAT_AS(LTMaterial, ambient.green, "ambient_green")
LT_REGISTER_FIELD_FLOAT_AS(LTMaterial, ambient.blue, "ambient_blue")
LT_REGISTER_FIELD_FLOAT_AS(LTMaterial, diffuse.red, "diffuse_red")
LT_REGISTER_FIELD_FLOAT_AS(LTMaterial, diffuse.green, "diffuse_green")
LT_REGISTER_FIELD_FLOAT_AS(LTMaterial, diffuse.blue, "diffuse_blue")
LT_REGISTER_FIELD_FLOAT_AS(LTMaterial, diffuse.alpha, "diffuse_alpha")
LT_REGISTER_FIELD_FLOAT_AS(LTMaterial, specular.red, "specular_red")
LT_REGISTER_FIELD_FLOAT_AS(LTMaterial, specular.green, "specular_green")
LT_REGISTER_FIELD_FLOAT_AS(LTMaterial, specular.blue, "specular_blue")
LT_REGISTER_FIELD_FLOAT_AS(LTMaterial, emission.red, "emission_red")
LT_REGISTER_FIELD_FLOAT_AS(LTMaterial, emission.green, "emission_green")
LT_REGISTER_FIELD_FLOAT_AS(LTMaterial, emission.blue, "emission_blue")
LT_REGISTER_FIELD_FLOAT(LTMaterial, shininess)
