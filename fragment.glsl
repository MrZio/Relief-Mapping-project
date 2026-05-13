#version 330 core
out vec4 FragColor;

in vec2 vTexCoord;
in vec3 vViewDirTangent;

uniform float uDepthScale;

// 1. GENERAZIONE DEL RILIEVO (Una formula matematica semplice e spiegabile)
float getHeight(vec2 uv) {
    // Moltiplichiamo per 15.0 per avere una griglia di colline
    vec2 p = uv * 15.0;
    // Seno * Coseno crea le gobbe. * 0.5 + 0.5 le tiene nel range [0.0, 1.0]
    return (sin(p.x) * cos(p.y)) * 0.5 + 0.5;
}

// 2. CALCOLO DELLA NORMALE (Differenze finite di base)
vec3 getNormal(vec2 uv) {
    float eps = 0.01;
    float hL = getHeight(uv + vec2(-eps, 0.0));
    float hR = getHeight(uv + vec2(eps, 0.0));
    float hD = getHeight(uv + vec2(0.0, -eps));
    float hU = getHeight(uv + vec2(0.0, eps));
    return normalize(vec3(hL - hR, hD - hU, 2.0 * eps));
}

void main() {
    vec3 v = normalize(vViewDirTangent);
    v.z = max(abs(v.z), 0.1); // Sicurezza contro la divisione per zero

    float numSteps = mix(40.0, 10.0, v.z);
    float deltaDepth = 1.0 / numSteps;
    vec2 deltaUV = (v.xy * uDepthScale) / (v.z * numSteps);

    vec2 currentUV = vTexCoord;
    float currentRayDepth = 0.0;

    // --- 1. RICERCA LINEARE ---
    for(int i = 0; i < int(numSteps); i++) {
        currentUV -= deltaUV;
        currentRayDepth += deltaDepth;
        if(currentRayDepth >= getHeight(currentUV)) break;
    }

    // --- 2. RICERCA BINARIA ---
    deltaUV *= 0.5;
    deltaDepth *= 0.5;
    currentUV += deltaUV;
    currentRayDepth -= deltaDepth;

    for(int j = 0; j < 5; j++) {
        deltaUV *= 0.5;
        deltaDepth *= 0.5;
        if(currentRayDepth > getHeight(currentUV)) {
            currentUV += deltaUV;
            currentRayDepth -= deltaDepth;
        } else {
            currentUV -= deltaUV;
            currentRayDepth += deltaDepth;
        }
    }

    // Taglio dei bordi per non sforare la faccia del cubo
    if(currentUV.x < 0.0 || currentUV.x > 1.0 || currentUV.y < 0.0 || currentUV.y > 1.0) {
        discard;
    }

    // --- 3. ILLUMINAZIONE BASE (Lambert) ---
    vec3 N = getNormal(currentUV);
    vec3 L = normalize(vec3(1.0, 1.0, 1.0)); // Luce fissa dall'alto a destra
    float diffuse = max(dot(N, L), 0.0);

    // Colore argilla di base moltiplicato per la luce
    vec3 color = vec3(0.7, 0.4, 0.3) * (diffuse + 0.2);
    FragColor = vec4(color, 1.0);
}