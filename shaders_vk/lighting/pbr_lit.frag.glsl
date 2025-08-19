#version 450
layout(location=0) in vec3 vNormal;
layout(location=1) in vec2 vUV;
layout(location=0) out vec4 fragColor;

layout(binding=0) uniform sampler2D uAlbedo;
layout(binding=1) uniform sampler2D uNormal;
layout(binding=2) uniform sampler2D uMetalRough;
layout(binding=3) uniform sampler2D uBrdfLUT;
layout(binding=4) uniform samplerCube uIrradiance;
#include "lighting/csm_binding.glsl"
#include "lighting/pbr_common.glsl"
#include "lighting/pcss.glsl"

layout(push_constant) uniform Push {
    mat4 model;
    mat4 viewProj;
    vec3 camPos;
} pc;

void main(){
    vec3 albedo = texture(uAlbedo, vUV).rgb;
    vec3 nTex = texture(uNormal, vUV).xyz * 2.0 - 1.0;
    vec3 N = normalize(mat3(pc.model) * nTex);
    vec3 V = normalize(pc.camPos - (pc.model * vec4(0.0,0.0,0.0,1.0)).xyz);
    vec3 L = normalize(vec3(0.5,1.0,0.3));
    vec3 H = normalize(L+V);
    float rough = texture(uMetalRough, vUV).g;
    float metal = texture(uMetalRough, vUV).b;
    float NoV = saturate(dot(N,V));
    float NoL = saturate(dot(N,L));
    float NoH = saturate(dot(N,H));
    float VoH = saturate(dot(V,H));
    vec3 F0 = mix(vec3(0.04), albedo, metal);
    vec3 F = F_Schlick(F0, VoH);
    float D = D_GGX(NoH, rough);
    float G = V_SmithGGX(NoV, NoL, rough);
    vec3 spec = (D*G) * F;
    vec3 kd = (vec3(1.0) - F) * (1.0 - metal);
    vec3 diff = kd * lambert(albedo);
    float shadow = pcss_visibility(uShadowMap, vec3(vUV,0.0), 1.0, 1.0, 1.0, 1.0, 0.0);
    vec3 direct = (diff + spec) * NoL * shadow;
    vec3 irr = texture(uIrradiance, N).rgb;
    vec2 brdf = texture(uBrdfLUT, vec2(NoV, rough)).rg;
    vec3 specIbl = irr * (F * brdf.x + brdf.y);
    vec3 color = direct + specIbl;
    fragColor = vec4(color, 1.0);
}
