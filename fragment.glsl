#version 330 core
out vec4 FragColor;

in vec2 vTexCoord;
in vec3 vViewDirTangent;

uniform float uDepthScale;

// 1. IL FRATTALE (Generazione procedurale del rilievo)
float getHeight(vec2 uv) {
    vec2 p = uv * 10.0;
    float altezza = 0.0;
    float ampiezza = 0.5;
    float frequenza = 1.0;

    for(int i = 0; i < 4; i++) {
        altezza += ampiezza * (sin(p.x * frequenza) * cos(p.y * frequenza));
        ampiezza *= 0.5;
        frequenza *= 2.0;
        p += vec2(1.7, 1.3); // Offset per rompere la simmetria
    }
    // Normalizziamo l'altezza tra 0.0 (fondo) e 1.0 (superficie)
    return altezza * 0.5 + 0.5;
}

// 2. LE NORMALI (Calcolo della pendenza per la luce)
vec3 getNormal(vec2 uv) {
    float eps = 0.01;

    float hL = getHeight(uv + vec2(-eps, 0.0));
    float hR = getHeight(uv + vec2(eps, 0.0));
    float hD = getHeight(uv + vec2(0.0, -eps));
    float hU = getHeight(uv + vec2(0.0, eps));

    vec3 normal = vec3(hL - hR, hD - hU, 2.0 * eps);
    return normalize(normal);
}

void main() {
    // --- SETUP DEL RAGGIO ---
    vec3 v = normalize(vViewDirTangent);

    // Limite di sicurezza per l'angolo radente (evita divisioni per zero)
    v.z = max(abs(v.z), 0.1);

    // Passi dinamici: 60 se guardiamo di lato, 15 se guardiamo di fronte
    float numSteps = mix(60.0, 15.0, v.z);

    float deltaDepth = 1.0 / numSteps;
    vec2 deltaUV = (v.xy * uDepthScale) / (v.z * numSteps);

    vec2 currentUV = vTexCoord;
    float currentRayDepth = 0.0;

    // --- LINEAR SEARCH (Discesa nel volume) ---
    for(int i = 0; i < int(numSteps); i++) {
        currentUV -= deltaUV; // MENO: Ci muoviamo verso l'interno della mappa
        currentRayDepth += deltaDepth;
        if(currentRayDepth >= getHeight(currentUV)) {
            break;
        }
    }

    // --- SETUP RICERCA BINARIA ---
    deltaUV *= 0.5;
    deltaDepth *= 0.5;
    currentUV += deltaUV; // PIÙ: Torniamo indietro di mezzo passo verso la superficie
    currentRayDepth -= deltaDepth;

    // --- BINARY SEARCH (Raffinamento dell'intersezione) ---
    for(int j = 0; j < 5; j++) {
        deltaUV *= 0.5;
        deltaDepth *= 0.5;
        if(currentRayDepth > getHeight(currentUV)) {
            // Se siamo "dentro" la collina, saliamo
            currentUV += deltaUV;
            currentRayDepth -= deltaDepth;
        } else {
            // Se siamo "fuori" all'aria aperta, scendiamo
            currentUV -= deltaUV;
            currentRayDepth += deltaDepth;
        }
    }

    // --- CLIPPING (Taglio dei bordi) ---
    // Se il raggio ha "bucato" il lato del cubo uscendo dalle UV, scarta il pixel
    if(currentUV.x < 0.0 || currentUV.x > 1.0 || currentUV.y < 0.0 || currentUV.y > 1.0) {
        discard;
    }

    // --- ILLUMINAZIONE (Modello Phong/Lambert) ---
    // 1. Pendenza
    vec3 N = getNormal(currentUV);

    // 2. Vettore Luce (da in alto a destra verso l'oggetto)
    vec3 L = normalize(vec3(1.0, 1.0, 1.0));

    // 3. Calcolo delle ombre diffuse
    float diffuse = max(dot(N, L), 0.0);
    float ambient = 0.2; // Luce di base

    // 4. Colore (es. legno chiaro / terracotta)
    vec3 baseColor = vec3(0.75, 0.55, 0.45);
    vec3 finalColor = baseColor * (diffuse + ambient);

    FragColor = vec4(finalColor, 1.0);
}