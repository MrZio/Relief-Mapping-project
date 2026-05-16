#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec3 aNormal;

uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uModel;
uniform vec3 uViewPos;
uniform vec3 uLightPos;

out vec2 vTexCoord;
out vec3 vViewDirTangent;
out vec3 vLightDirTangent;
out vec3 vNormalWorld;
out vec3 vTangentWorld;

void main() {
    vTexCoord = aTexCoord;

    // TBN in world space
    vec3 T = normalize(vec3(uModel * vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(uModel * vec4(aNormal,  0.0)));
    vec3 B = cross(N, T);

    vNormalWorld  = N;
    vTangentWorld = T;

    // TBN trasposta = da world a tangent space
    mat3 TBN = transpose(mat3(T, B, N));

    vec3 worldPos = vec3(uModel * vec4(aPos, 1.0));

    vViewDirTangent  = TBN * normalize(uViewPos  - worldPos);
    vLightDirTangent = TBN * normalize(uLightPos - worldPos);

    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}
