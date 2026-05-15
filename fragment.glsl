#version 330 core
out vec4 FragColor;

in vec2 vTexCoord;
in vec3 vViewDirTangent;
in vec3 vLightDirTangent;

uniform float uDepthScale;
uniform int   uSearchMode;      // 0 = solo linear, 1 = linear + binary
uniform sampler2D uHeightMap;
uniform sampler2D uDiffuseMap;

// ============================================================
//  HEIGHT: legge dalla texture e normalizza il contrasto
//  La heightmap del canyon ha valori ~[0.40, 0.65] invece di [0,1]
//  Stretchiamo per usare tutto il range disponibile.
// ============================================================

float getHeight(vec2 uv) {
    return 1.0 - texture(uHeightMap, uv).r;
}

// ============================================================
//  NORMALE per differenze finite
// ============================================================
vec3 getNormal(vec2 uv) {
    float eps = 0.003;
    float hL = getHeight(uv + vec2(-eps, 0.0));
    float hR = getHeight(uv + vec2( eps, 0.0));
    float hD = getHeight(uv + vec2(0.0, -eps));
    float hU = getHeight(uv + vec2(0.0,  eps));
    // strength controlla quanto accentuare le ombre sui dettagli
    float strength = 6.0;
    return normalize(vec3((hL - hR) * strength, (hD - hU) * strength, 1.0));
}

// ============================================================
//  POM: linear oppure linear + binary  (toggle via uSearchMode)
// ============================================================
vec2 reliefTrace(vec2 uv, vec3 viewDir) {
    float vz = max(abs(viewDir.z), 0.04);

    // Passi adattativi: più passi quando la vista è radente
    float numSteps = mix(96.0, 16.0, vz * vz);
    float deltaDepth = 1.0 / numSteps;
    vec2  deltaUV    = (viewDir.xy * uDepthScale) / (vz * numSteps);

    vec2  curUV   = uv;
    float curD    = 0.0;
    vec2  prevUV  = uv;
    float prevD   = 0.0;

    // --- FASE 1: ricerca lineare ---
    for (int i = 0; i < 96; i++) {
        if (float(i) >= numSteps) break;
        prevUV = curUV;
        prevD  = curD;
        curUV -= deltaUV;
        curD  += deltaDepth;
        if (curD >= getHeight(curUV)) break;
    }

    if (uSearchMode == 0) {
        // Solo linear: interpolazione lineare tra i due passi contigui
        float after  = getHeight(curUV)  - curD;
        float before = getHeight(prevUV) - prevD;
        float t = after / (after - before + 0.0001);
        return mix(curUV, prevUV, t);
    }

    // --- FASE 2: binary search (6 passi) ---
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
//  SELF-SHADOW: raggio verso la luce dalla superficie
// ============================================================
float pomShadow(vec2 uv, float surfaceH, vec3 lightDir) {
    float lz = max(abs(lightDir.z), 0.04);
    float numSteps   = mix(32.0, 8.0, lz * lz);
    float layerStep  = (1.0 - surfaceH) / numSteps;
    vec2  uvStep     = (lightDir.xy * uDepthScale) / (lz * numSteps);
    vec2  curUV      = uv;
    float curD       = surfaceH;
    float shadow     = 0.0;
    for (int i = 0; i < 32; i++) {
        if (float(i) >= numSteps) break;
        curUV += uvStep;
        curD  += layerStep;
        float h = getHeight(curUV);
        if (h > curD) shadow = max(shadow, (h - curD) * 5.0);
    }
    return 1.0 - clamp(shadow, 0.0, 0.75);
}

// ============================================================
//  MAIN
// ============================================================
void main() {
    vec3 V = normalize(vViewDirTangent);
    vec3 L = normalize(vLightDirTangent);

    // 1. Trova il punto di intersezione (Ricerca Lineare + Binaria)
    // Ricordati di rinominare pomTrace in reliefTrace nel codice sopra!
    vec2 dispUV = reliefTrace(vTexCoord, V);

    // 2. Paracadute per i bordi (il discard interviene solo se sforiamo la maschera edge)
    if (dispUV.x < 0.0 || dispUV.x > 1.0 || dispUV.y < 0.0 || dispUV.y > 1.0) {
            discard;
        }

        float h = getHeight(dispUV);

        // 2. LA MAGIA IBO: Scarta lo sfondo nero di Blender!
        // Se l'altezza è quasi 1.0, significa che il raggio ha colpito lo sfondo vuoto.
        if (h > 0.85) {
            discard;
        }

        // 3. Usa la Normal Map reale! (che nel C++ abbiamo legato a uDiffuseMap)
        vec3 rawNormal = texture(uDiffuseMap, dispUV).rgb;
        // Decodifica da [0, 1] a [-1, 1]
        vec3 N = normalize(rawNormal * 2.0 - 1.0);

        // 4. Colore base (Diamo a Suzanne un colore solido stile "gesso")
        vec3 baseColor = vec3(0.8, 0.8, 0.8);

        // 5. Illuminazione Lambert standard
        float ambient = 0.15;
        float diffuse = max(dot(N, L), 0.0);

        vec3 finalColor = baseColor * (ambient + diffuse);
        finalColor = pow(clamp(finalColor, 0.0, 1.0), vec3(1.0 / 2.2));

        FragColor = vec4(finalColor, 1.0);
}
