#version 330 core
out vec4 FragColor;

in vec2 vTexCoord;
in vec3 vViewDirTangent;
in vec3 vLightDirTangent;
in vec3 vNormalWorld;
in vec3 vTangentWorld;

uniform float uDepthScale;
uniform int   uSearchMode;
uniform int   uLinearSteps;
uniform int   uUseDualDepth;
uniform sampler2D uHeightMap;    // front depth
uniform sampler2D uDiffuseMap;   // normal map
uniform sampler2D uDepthBackMap; // back depth

const float BG_RAW  = 0.30;
const float RAW_MAX = 0.97;

// Altezza FRONTE in [0,1]: 0 = vicino, 1 = lontano
float getHeight(vec2 uv) {
    uv = clamp(uv, vec2(0.0), vec2(1.0));
    float raw = texture(uHeightMap, uv).r;
    if (raw < BG_RAW) return 1.0;
    return clamp((RAW_MAX - raw) / (RAW_MAX - BG_RAW), 0.0, 1.0);
}

// Altezza RETRO in [0,1], stesso spazio del fronte
float getBackHeight(vec2 uv) {
    uv = clamp(uv, vec2(0.0), vec2(1.0));
    float raw = texture(uDepthBackMap, uv).r;
    if (raw < BG_RAW) return 1.0;
    return clamp((RAW_MAX - raw) / (RAW_MAX - BG_RAW), 0.0, 1.0);
}

bool isBackground(vec2 uv) {
    uv = clamp(uv, vec2(0.0), vec2(1.0));
    return texture(uHeightMap, uv).r < BG_RAW;
}

// ============================================================
//  RELIEF TRACE
//  Trova il primo impatto sul fronte (linear + binary).
//  Il test dual-depth è applicato come VALIDAZIONE del punto finale,
//  non durante la marcia (così non mangia orecchie/colli sottili).
// ============================================================
vec2 reliefTrace(vec2 uv, vec3 viewDir, out bool hit) {
    hit = false;
    float vz = max(abs(viewDir.z), 0.04);

    // Passi adattativi: piu' passi quando la vista e' radente (vz piccolo)
    // Questo riduce i "puntini" agli angoli estremi.
    float baseSteps = float(max(uLinearSteps, 4));
    float numSteps  = mix(baseSteps * 2.5, baseSteps, vz);  // fino a 2.5x ai bordi
    numSteps = min(numSteps, 128.0);

    float deltaDepth = 1.0 / numSteps;
    vec2  deltaUV    = (viewDir.xy * uDepthScale) / (vz * numSteps);

    vec2  curUV  = uv,  prevUV = uv;
    float curD   = 0.0, prevD  = 0.0;

    for (int i = 0; i < 128; i++) {
        if (float(i) >= numSteps) break;

        prevUV = curUV;
        prevD  = curD;
        curUV -= deltaUV;
        curD  += deltaDepth;

        if (curUV.x < 0.0 || curUV.x > 1.0 ||
            curUV.y < 0.0 || curUV.y > 1.0) {
            hit = false;
            break;
        }

        if (curD >= getHeight(curUV)) {
            hit = true;
            break;
        }
    }

    if (!hit) return uv;

    if (uSearchMode == 0) return curUV;

    // Binary search refinement
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

void main() {
    vec3 V = normalize(vViewDirTangent);
    vec3 L = normalize(vLightDirTangent);

    bool hit;
    vec2 dispUV = reliefTrace(vTexCoord, V, hit);

    if (!hit) discard;
    if (isBackground(dispUV)) discard;

    // ============================================================
    //  VALIDAZIONE DUAL-DEPTH (anti-skin)
    //  La skin/allungamento si riconosce perché il punto trovato (dispUV)
    //  è MOLTO lontano, in UV, dal punto di partenza (vTexCoord):
    //  il raggio è "scivolato" lungo una superficie inclinata.
    //  Combiniamo due test:
    //   1) il retro a dispUV non deve essere sfondo (guscio reale)
    //   2) lo scivolamento UV non deve superare lo spessore del guscio
    // ============================================================
    if (uUseDualDepth == 1) {
        // Test 1: il guscio deve esistere (retro non sfondo)
        float rawB = texture(uDepthBackMap, clamp(dispUV, 0.0, 1.0)).r;
        if (rawB < BG_RAW) discard;

        // Test 2: spessore del guscio
        float hF = getHeight(dispUV);
        float hB = getBackHeight(dispUV);
        float shell = hB - hF;

        float slide = length(dispUV - vTexCoord);

        // Soglia slide proporzionale all'angolo di vista (radente = piu' permissivo)
        float vz = max(abs(V.z), 0.04);
        float slideThreshold = mix(0.30, 0.06, vz);

        // E ANCHE proporzionale alla profondita': con depth alto i raggi
        // scivolano di piu' fisiologicamente, quindi alziamo la soglia
        // per non svuotare orecchie e dettagli.
        slideThreshold += uDepthScale * 0.5;

        // Scarta solo skin vere
        if (shell < 0.04 && slide > slideThreshold) discard;
    }

    // Normal map
    vec3 N = normalize(texture(uDiffuseMap, dispUV).rgb * 2.0 - 1.0);

    // Phong
    float ambient = 0.20;
    float diffuse = max(dot(N, L), 0.0);
    float spec    = pow(max(dot(reflect(-L, N), V), 0.0), 32.0) * 0.3;

    vec3 color = vec3(0.85, 0.83, 0.80) * (ambient + diffuse) + vec3(spec);
    color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
