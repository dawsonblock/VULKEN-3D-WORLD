
#ifndef PCSS_GLSL
#define PCSS_GLSL
float pcss_visibility(sampler2DArray sm, vec3 uvw, float texel, float searchPx, float minPx, float maxPx, float bias)
{
    float receiver = uvw.z - bias;
    // Blocker search
    float r = searchPx * texel;
    float avg=0.0; int blockers=0;
    const int K=16;
    for(int i=0;i<K;i++){
        float a = 6.2831853 * (i/float(K));
        vec2 o = vec2(cos(a), sin(a)) * r * float((i%4)+1) / 4.0;
        float d = texture(sm, vec3(uvw.xy + o, uvw.z)).r;
        if(d < receiver){ avg += d; blockers++; }
    }
    if(blockers==0) return 1.0;
    avg /= float(blockers);
    float pen = clamp((receiver - avg) * 200.0, minPx, maxPx); // 200 tuned to typical scene scale
    // PCF with variable radius
    const int S=24;
    int lit=0;
    for(int i=0;i<S;i++){
        float a = 6.2831853 * (i/float(S));
        vec2 o = vec2(cos(a), sin(a)) * pen * texel;
        float d = texture(sm, vec3(uvw.xy + o, uvw.z)).r;
        lit += int(d + 1e-4 >= receiver);
    }
    return float(lit)/float(S);
}
#endif
