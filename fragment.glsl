#version 330 core
out vec4 FragColor;

in vec2 vTexCoord;
in vec3 vViewDirTangent;
in vec3 vLightDirTangent;
in vec3 vNormalWorld;
in vec3 vTangentWorld;

uniform float uDepthScale;
uniform int   uSearchMode;    // 0 = linear only, 1 = linear + binary
uniform int   uLinearSteps;   // numero di passi lineari (slider, default 16)
uniform sampler2D uHeightMap;
uniform sampler2D uDiffuseMap;

const float BG_THRESHOLD = 0.44;

float getHeight(vec2 uv) {
    float raw = texture(uHeightMap, uv).r;
    float h   = 1.0 - raw;
    float ex  = smoothstep(0.0, 0.04, uv.x) * (1.0 - smoothstep(0.96, 1.0, uv.x));
    float ey  = smoothstep(0.0, 0.04, uv.y) * (1.0 - smoothstep(0.96, 1.0, uv.y));
    return h * ex * ey;
}

// ============================================================
//  RELIEF TRACE
//
//  Modalità 0 — LINEAR ONLY
//    Il raggio avanza a passi uniformi lungo viewDir.
//    Si ferma al primo passo che supera la superficie.
//    Risultato: il punto di intersezione è quello grezzo dell'ultimo
//    passo → con pochi passi si vedono chiaramente le "scalinature"
//    (banding) ai bordi e nelle zone ad alto contrasto del rilievo.
//    Questo è l'artefatto classico della sola ricerca lineare.
//
//  Modalità 1 — LINEAR + BINARY SEARCH
//    Dopo la fase lineare (stessi passi), si eseguono 6 iterazioni
//    di binary search nell'intervallo [prevUV, curUV], dimezzando
//    ogni volta l'intervallo di incertezza.
//    6 iterazioni → precisione equivalente a 2^6 = 64 volte più fine
//    del passo lineare, senza campionare 64 volte in più.
//    Risultato: bordi precisi, nessun banding visibile.
// ============================================================
vec2 reliefTrace(vec2 uv, vec3 viewDir) {
    float vz = max(abs(viewDir.z), 0.04);

    // Usiamo uLinearSteps (controllato da slider) invece di un valore fisso.
    // Con 16 passi le scalinature della modalità 0 sono chiaramente visibili.
    // Con 64+ passi la differenza è quasi impercettibile a occhio nudo.
    float numSteps   = float(max(uLinearSteps, 4));
    float deltaDepth = 1.0 / numSteps;
    vec2  deltaUV    = (viewDir.xy * uDepthScale) / (vz * numSteps);

    vec2  curUV  = uv,  prevUV = uv;
    float curD   = 0.0, prevD  = 0.0;

    // --- FASE LINEARE (comune a entrambe le modalità) ---
    for (int i = 0; i < 128; i++) {
        if (float(i) >= numSteps) break;
        prevUV = curUV;  prevD = curD;
        curUV -= deltaUV;
        curD  += deltaDepth;
        float h = getHeight(curUV);
        if (curD >= h && h < BG_THRESHOLD) break;
    }

    // --- MODALITÀ 0: linear only ---
    // Ritorna il punto grezzo — nessun raffinamento.
    // Con uLinearSteps basso (es. 16) vedrai banding evidente.
    if (uSearchMode == 0) {
        return curUV;
    }

    // --- MODALITÀ 1: binary search refinement ---
    // Dimezza 6 volte l'intervallo tra l'ultimo passo valido (prevUV)
    // e il primo passo invalido (curUV). Precisione: 1/numSteps / 2^6.
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

    vec2 dispUV = reliefTrace(vTexCoord, V);

    if (dispUV.x < 0.0 || dispUV.x > 1.0 ||
        dispUV.y < 0.0 || dispUV.y > 1.0) discard;

    float h = getHeight(dispUV);

    // Anti-speckling: neighbourhood check
    float eps = 0.004;
    float avgH = (getHeight(dispUV + vec2( 0.0,  eps)) +
                  getHeight(dispUV + vec2( 0.0, -eps)) +
                  getHeight(dispUV + vec2( eps,  0.0)) +
                  getHeight(dispUV + vec2(-eps,  0.0))) * 0.25;

    if (h   >= BG_THRESHOLD) discard;
    if (avgH >= BG_THRESHOLD && h > BG_THRESHOLD * 0.85) discard;

    float edgeFade = 1.0 - smoothstep(BG_THRESHOLD * 0.80, BG_THRESHOLD * 0.98, h);

    // Normal map
    vec3 N = normalize(texture(uDiffuseMap, dispUV).rgb * 2.0 - 1.0);

    // Phong
    float ambient = 0.18;
    float diffuse = max(dot(N, L), 0.0);
    float spec    = pow(max(dot(reflect(-L, N), V), 0.0), 32.0) * 0.3;

    vec3 color = vec3(0.85, 0.83, 0.80) * (ambient + diffuse) + vec3(spec);
    color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / 2.2));

    FragColor = vec4(color, edgeFade);
}
