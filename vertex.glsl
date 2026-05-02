#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec3 aNormal;

uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uModel;
uniform vec3 uViewPos;

out vec2 vTexCoord;
out vec3 vViewDirTangent;

void main() {
    vTexCoord = aTexCoord;

    // 1. Ruotiamo Tangente e Normale nello spazio del mondo (World Space)
    // Moltiplichiamo per uModel. Usiamo 0.0 perché sono direzioni, non punti spaziali.
    vec3 T = normalize(vec3(uModel * vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(uModel * vec4(aNormal, 0.0)));

    // 2. Calcoliamo la Bitangente con i vettori ruotati
    vec3 B = cross(N, T);

    // 3. Ora la matrice TBN girerà perfettamente insieme al tuo cubo!
    mat3 tbn = transpose(mat3(T, B, N));

    // Posizione del vertice nel mondo
    vec3 worldPos = vec3(uModel * vec4(aPos, 1.0));

    // Vettore dalla telecamera al vertice
    vec3 viewDirWorld = normalize(uViewPos - worldPos);

    // Trasformazione finale nello Spazio Tangente
    vViewDirTangent = tbn * viewDirWorld;

    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}