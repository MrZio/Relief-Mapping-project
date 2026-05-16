#version 330 core
out vec4 FragColor;

in vec2 vTexCoord;
in vec3 vViewDirTangent;
in vec3 vLightDirTangent;
in vec3 vNormalWorld;       // normale della faccia del cubo in world space
in vec3 vTangentWorld;      // tangente in world space

uniform float uDepthScale;
uniform int   uSearchMode;
uniform sampler2D uHeightMap;   // depth map da Blender (bianco=vicino, nero=lontano)
uniform sampler2D uDiffuseMap;  // normal map da Blender (World Space)

// ============================================================
//  DEPTH MAP: Blender esporta bianco=vicino (depth piccola),
//  nero=lontano (depth grande), sfondo = grigio medio (~0.5)
//
//  Vogliamo: 0.0 = superficie = punto più vicino (bianco in Blender)
//            1.0 = fondo del volume = punto più lontano (nero in Blender)
//
//  Quindi: h = 1.0 - raw   (invertiamo)
//  Poi normalizziamo escludendo lo sfondo grigio.
//  Lo sfondo di Blender è ~0.53 (grigio medio) → dopo inversione = ~0.47
//  L'oggetto va da 0.0 (superficie) fino a ~0.45 circa
//  Usiamo 0.48 come soglia di taglio sfondo.
// ============================================================
float getHeight(vec2 uv) {
    float raw = texture(uHeightMap, uv).r;
    // Inverti: ora bianco Blender → 0.0, nero Blender → 1.0
    float h = 1.0 - raw;

    // Vignette ai bordi del quad per chiudere i bordi del cubo
    float ex = smoothstep(0.0, 0.04, uv.x) * (1.0 - smoothstep(0.96, 1.0, uv.x));
    float ey = smoothstep(0.0, 0.04, uv.y) * (1.0 - smoothstep(0.96, 1.0, uv.y));
    return h * ex * ey;
}

// Soglia sfondo: tutto ciò che supera questo valore è sfondo Blender
// Il grigio medio di Blender (~0.53) dopo inversione → ~0.47
// Usiamo 0.46 per stare sicuri (un po' sotto)
const float BG_THRESHOLD = 0.46;

// ============================================================
//  RELIEF TRACE: linear + optional binary
// ============================================================
vec2 reliefTrace(vec2 uv, vec3 viewDir) {
    float vz = max(abs(viewDir.z), 0.04);
    float numSteps   = mix(96.0, 16.0, vz * vz);
    float deltaDepth = 1.0 / numSteps;
    // NOTA: sottraiamo deltaUV perché ci muoviamo "dentro" il volume
    vec2  deltaUV    = (viewDir.xy * uDepthScale) / (vz * numSteps);

    vec2  curUV  = uv,  prevUV = uv;
    float curD   = 0.0, prevD  = 0.0;

    for (int i = 0; i < 96; i++) {
        if (float(i) >= numSteps) break;
        prevUV = curUV;  prevD = curD;
        curUV -= deltaUV;
        curD  += deltaDepth;
        float h = getHeight(curUV);
        // Interrompi solo se colpiamo geometria reale (non sfondo)
        if (curD >= h && h < BG_THRESHOLD) break;
    }

    if (uSearchMode == 0) {
        float after  = getHeight(curUV)  - curD;
        float before = getHeight(prevUV) - prevD;
        float t = after / (after - before + 0.0001);
        return mix(curUV, prevUV, t);
    }

    // Binary search
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

// ============================================================
//  MAIN
// ============================================================
void main() {
    vec3 V = normalize(vViewDirTangent);
    vec3 L = normalize(vLightDirTangent);

    // 1. POM trace
    vec2 dispUV = reliefTrace(vTexCoord, V);

    // 2. Scarta pixel fuori dal quad
    if (dispUV.x < 0.0 || dispUV.x > 1.0 ||
        dispUV.y < 0.0 || dispUV.y > 1.0) discard;

    // 3. Leggi l'altezza finale e scarta lo sfondo di Blender
    float h = getHeight(dispUV);
    if (h >= BG_THRESHOLD) discard;   // sfondo grigio di Blender → trasparente

    // 4. Normal map
    // La normal map di Blender è in Object/World Space con encoding [0,1]→[-1,1]
    // Decodifica standard:
    vec3 rawN = texture(uDiffuseMap, dispUV).rgb;
    vec3 N    = normalize(rawN * 2.0 - 1.0);

    // La normal map di Blender usa convenzione OpenGL (Y verde = su).
    // Se le luci sembrano invertite verticalmente, decommentare la riga sotto:
    // N.y = -N.y;

    // Le normali di Blender Object Space NON sono nello spazio tangente del cubo,
    // ma per l'IBO questo VA BENE: la normal map cattura la geometria 3D dell'oggetto
    // vista dalla direzione della faccia. Usiamo la normale direttamente in
    // tangent space (il sistema di riferimento della faccia del cubo = lo spazio
    // in cui la normal map è già espressa correttamente per quella vista).

    // 5. Illuminazione Lambert + Phong
    float ambient = 0.18;
    float diffuse = max(dot(N, L), 0.0);

    // Specular
    vec3  R    = reflect(-L, N);
    float spec = pow(max(dot(R, V), 0.0), 32.0) * 0.3;

    // Colore base (grigio "gesso" per mostrare bene le normali)
    vec3 baseColor = vec3(0.85, 0.83, 0.80);

    vec3 color = baseColor * (ambient + diffuse) + vec3(1.0) * spec;

    // Correzione gamma
    color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
