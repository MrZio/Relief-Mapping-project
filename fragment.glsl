#version 330 core
out vec4 FragColor;

in vec2 vTexCoord;
in vec3 vViewDirTangent;
in vec3 vLightDirTangent;
in vec3 vNormalWorld;
in vec3 vTangentWorld;

uniform float uDepthScale;
uniform int   uSearchMode;
uniform sampler2D uHeightMap;
uniform sampler2D uDiffuseMap;

// ============================================================
//  DEPTH MAP
//  Blender: bianco=vicino, nero=lontano, sfondo=grigio ~0.53
//  Dopo inversione: sfondo → ~0.47, oggetto → [0, ~0.44]
// ============================================================
const float BG_THRESHOLD = 0.44;   // leggermente abbassato per sicurezza

float getHeight(vec2 uv) {
    float raw = texture(uHeightMap, uv).r;
    float h   = 1.0 - raw;

    // Vignette ai bordi del quad
    float ex = smoothstep(0.0, 0.04, uv.x) * (1.0 - smoothstep(0.96, 1.0, uv.x));
    float ey = smoothstep(0.0, 0.04, uv.y) * (1.0 - smoothstep(0.96, 1.0, uv.y));
    return h * ex * ey;
}

// ============================================================
//  RELIEF TRACE
// ============================================================
vec2 reliefTrace(vec2 uv, vec3 viewDir) {
    float vz = max(abs(viewDir.z), 0.04);
    float numSteps   = mix(96.0, 16.0, vz * vz);
    float deltaDepth = 1.0 / numSteps;
    vec2  deltaUV    = (viewDir.xy * uDepthScale) / (vz * numSteps);

    vec2  curUV  = uv,  prevUV = uv;
    float curD   = 0.0, prevD  = 0.0;

    for (int i = 0; i < 96; i++) {
        if (float(i) >= numSteps) break;
        prevUV = curUV;  prevD = curD;
        curUV -= deltaUV;
        curD  += deltaDepth;
        float h = getHeight(curUV);
        if (curD >= h && h < BG_THRESHOLD) break;
    }

    if (uSearchMode == 0) {
        float after  = getHeight(curUV)  - curD;
        float before = getHeight(prevUV) - prevD;
        float t = after / (after - before + 0.0001);
        return mix(curUV, prevUV, t);
    }

    vec2  lo = prevUV; float loD = prevD;
    vec2  hi = curUV;  float hiD = curD;
    for (int j = 0; j < 6; j++) {
        vec2  mid  = (lo + hi)   * 0.5;
        float midD = (loD + hiD) * 0.5;
        if (midD < getHeight(mid)) { lo = mid; loD = midD; }
        else                       { hi = mid; hiD = midD; }
    }
    return (lo + hi) * 0.5;
}

// ===========================
//  MAIN
// ===========================
void main() {
    vec3 V = normalize(vViewDirTangent);
    vec3 L = normalize(vLightDirTangent);

    vec2 dispUV = reliefTrace(vTexCoord, V);

    // Bordi quad
    if (dispUV.x < 0.0 || dispUV.x > 1.0 ||
        dispUV.y < 0.0 || dispUV.y > 1.0) discard;

    float h = getHeight(dispUV);

    // --- FIX  for SPECKLING ---
    // 1. Leggiamo l'altezza anche in un intorno 2x2 vicino al pixel
    //    Se tutti i vicini sono sfondo, scartiamo anche noi
    float eps = 0.004;
    float hN  = getHeight(dispUV + vec2( 0.0,  eps));
    float hS  = getHeight(dispUV + vec2( 0.0, -eps));
    float hE  = getHeight(dispUV + vec2( eps,  0.0));
    float hW  = getHeight(dispUV + vec2(-eps,  0.0));

    // Se questo pixel è "oggetto" ma tutti i vicini sono sfondo → puntino isolato
    // oppure se l'altezza media dei vicini è sfondo → scarta
    float avgH = (hN + hS + hE + hW) * 0.25;

    // 2. Scarta: (a) sfondo diretto, (b) puntini isolati (pixel oggetto circondato da sfondo)
    if (h >= BG_THRESHOLD) discard;
    if (avgH >= BG_THRESHOLD && h > BG_THRESHOLD * 0.85) discard;

    // 3. Smooth alpha ai bordi dell'oggetto: invece di un discard netto,
    //    usiamo una zona di transizione soft basata su quanto siamo vicini alla soglia
    float edgeFade = 1.0 - smoothstep(BG_THRESHOLD * 0.80, BG_THRESHOLD * 0.98, h);

    // Normal map
    vec3 rawN = texture(uDiffuseMap, dispUV).rgb;
    vec3 N    = normalize(rawN * 2.0 - 1.0);

    // Illuminazione
    float ambient = 0.18;
    float diffuse = max(dot(N, L), 0.0);
    vec3  R       = reflect(-L, N);
    float spec    = pow(max(dot(R, V), 0.0), 32.0) * 0.3;

    vec3 baseColor = vec3(0.85, 0.83, 0.80);
    vec3 color     = baseColor * (ambient + diffuse) + vec3(1.0) * spec;
    color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / 2.2));

    // Applica il fade ai bordi (alpha morbido invece di discard secco)
    FragColor = vec4(color, edgeFade);
}
