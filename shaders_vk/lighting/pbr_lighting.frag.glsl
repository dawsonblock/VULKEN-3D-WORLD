#version 460
layout(location=0) in vec3 vWorldPos;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vUV;
layout(location=0) out vec4 outColor;

#include "lighting/pbr_common.glsl"
#include "shadows/csm_common.glsl"
#include "lighting/pcss.glsl"
#include "lighting/csm_binding.glsl"

layout(binding=0) uniform sampler2D uAlbedo;
layout(binding=1) uniform sampler2D uMetalRough;

layout(push_constant) uniform PC {
    mat4 model;
    mat4 viewProj;
    vec3 camPos;
    float _pad0;
    vec3 lightDir;
    float _pad1;
} pc;

float computeShadowVisibility(vec3 worldPos, float viewSpaceZ){
    int cas = chooseCascade(viewSpaceZ);
    vec4 lp = uLightViewProj[cas] * vec4(worldPos,1.0);
    vec3 uvw = lp.xyz / lp.w;
    uvw.xy = uvw.xy * 0.5 + 0.5;
    if(any(lessThan(uvw.xy, vec2(0.0))) || any(greaterThan(uvw.xy, vec2(1.0)))) return 1.0;
    return pcss_visibility(uShadowMap, vec3(uvw.xy, cas), uMapTexelSize, uPCSSSearch, uPCSSMin, uPCSSMax, 0.0008);
}

void main(){
    vec3 N = normalize(vNormal);
    vec3 V = normalize(pc.camPos - vWorldPos);
    vec3 L = normalize(-pc.lightDir);
    vec3 albedo = texture(uAlbedo, vUV).rgb;
    vec2 mr = texture(uMetalRough, vUV).rg;
    float metallic = mr.r;
    float roughness = mr.g;
    float NoL = saturate(dot(N,L));
    float NoV = saturate(dot(N,V));
    vec3 H = normalize(L+V);
    float NoH = saturate(dot(N,H));
    float VoH = saturate(dot(V,H));
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    float a = roughness*roughness;
    vec3 F = F_Schlick(F0, VoH);
    float D = D_GGX(NoH, a);
    float G = V_SmithGGX(NoV, NoL, a);
    vec3 spec = (D*G*F)/(4.0*NoL*NoV+1e-4);
    vec3 diff = lambert(albedo)*(1.0-metallic);
    float vis = computeShadowVisibility(vWorldPos, length(V));
    vec3 color = (diff + spec) * NoL * vis;
    outColor = vec4(color,1.0);
}
