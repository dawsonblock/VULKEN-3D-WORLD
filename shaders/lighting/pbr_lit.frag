#version 460
layout(location=0) in vec3 vWorldPos;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vUV;

layout(binding=1) uniform sampler2D uAlbedo;
layout(binding=2) uniform sampler2D uMetalRough;
layout(binding=4) uniform samplerCube uIrradiance;
layout(binding=7) uniform samplerCube uPrefiltered;

#include "shadows/csm_common.glsl"
#include "lighting/pbr_common.glsl"
#include "lighting/pcss.glsl"
#include "lighting/ibl.glsl"

layout(std140, binding=0) uniform Camera {
    mat4 uModel;
    mat4 uView;
    mat4 uProj;
} camera;

layout(push_constant) uniform Push { vec3 lightDir; vec3 lightColor; vec3 camPos; } pc;

layout(location=0) out vec4 oColor;

float computeShadow(vec3 worldPos){
    float viewZ = (camera.uView * vec4(worldPos,1.0)).z;
    int cas = chooseCascade(viewZ);
    vec4 lp = uLightViewProj[cas] * vec4(worldPos,1.0);
    vec3 uvw = lp.xyz / lp.w;
    uvw.xy = uvw.xy * 0.5 + 0.5;
    if(any(lessThan(uvw.xy, vec2(0.0))) || any(greaterThan(uvw.xy, vec2(1.0)))) return 1.0;
    return pcss_visibility(uShadowMap, vec3(uvw.xy, cas),
                           uMapTexelSize, uPCSSSearch, uPCSSMin, uPCSSMax, 0.0008);
}

void main(){
    vec3 albedo = texture(uAlbedo, vUV).rgb;
    vec2 mr = texture(uMetalRough, vUV).rg;
    float metallic = mr.x;
    float roughness = mr.y;

    vec3 N = normalize(vNormal);
    vec3 V = normalize(pc.camPos - vWorldPos);
    vec3 L = normalize(-pc.lightDir);
    vec3 H = normalize(V + L);

    float NoV = max(dot(N,V), 0.0);
    float NoL = max(dot(N,L), 0.0);
    float NoH = max(dot(N,H), 0.0);
    float LoH = max(dot(L,H), 0.0);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = F_Schlick(F0, LoH);
    float D = D_GGX(NoH, roughness);
    float G = V_SmithGGX(NoV, NoL, roughness);

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    vec3 spec = (D * G * F) / (4.0 * NoV * NoL + 1e-6);
    vec3 diff = lambert(albedo);

    float shadow = computeShadow(vWorldPos);
    vec3 radiance = pc.lightColor * NoL * shadow;
    vec3 Lo = (kD * diff + spec) * radiance;

    vec3 specIBL = ibl_specular(N, V, F0, roughness, uPrefiltered, 4.0);
    vec3 diffIBL = texture(uIrradiance, N).rgb * albedo;

    oColor = vec4(Lo + kS * specIBL + kD * diffIBL, 1.0);
}
