#version 450

#include "lighting/pbr_common.glsl"
#include "lighting/pcss.glsl"
#include "shadows/csm_common.glsl"
#include "lighting/csm_binding.glsl"

layout(set = 0, binding = 0) uniform sampler2D uAlbedo;
layout(set = 0, binding = 1) uniform sampler2D uMetalRough;
layout(set = 1, binding = 0) uniform sampler2D uBRDFLUT;

layout(push_constant) uniform PushConsts {
    mat4 view;
    vec3 camPos;
    float _pad0;
    vec3 lightDir;
    float _pad1;
} pc;

layout(location = 0) in vec3 vWorldPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUv;
layout(location = 0) out vec4 outColor;

float computeShadow(vec3 worldPos, float viewSpaceZ){
    int cas = chooseCascade(viewSpaceZ);
    vec4 lp = uLightViewProj[cas] * vec4(worldPos,1.0);
    vec3 uvw = lp.xyz / lp.w;
    uvw.xy = uvw.xy * 0.5 + 0.5;
    if(any(lessThan(uvw.xy, vec2(0.0))) || any(greaterThan(uvw.xy, vec2(1.0))))
        return 1.0;
    return pcss_visibility(uShadowMap, vec3(uvw.xy, cas),
                           uMapTexelSize, uPCSSSearch, uPCSSMin, uPCSSMax, SHADOW_BIAS);
}

void main(){
    vec3 N = normalize(vNormal);
    vec3 V = normalize(pc.camPos - vWorldPos);
    vec3 L = normalize(-pc.lightDir);
    vec3 H = normalize(V + L);

    vec3 albedo = texture(uAlbedo, vUv).rgb;
    vec2 mr = texture(uMetalRough, vUv).rg;
    float metallic = mr.r;
    float roughness = mr.g;

    float NoV = saturate(dot(N,V));
    float NoL = saturate(dot(N,L));
    float NoH = saturate(dot(N,H));
    float VoH = saturate(dot(V,H));

    float a = roughness * roughness;
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = F_Schlick(F0, VoH);
    float D = D_GGX(NoH, a);
    float VTerm = V_SmithGGX(NoV, NoL, a);
    vec3 spec = (D * VTerm) * F;
    vec3 kd = (1.0 - F) * (1.0 - metallic);
    vec3 diffuse = kd * lambert(albedo);

    float viewZ = abs((pc.view * vec4(vWorldPos,1.0)).z);
    float shadow = computeShadow(vWorldPos, viewZ);

    vec3 color = (diffuse + spec) * NoL * shadow;
    outColor = vec4(color, 1.0);
}

