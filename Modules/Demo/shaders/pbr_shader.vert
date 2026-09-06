#version 450

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoords;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Bitangent;

layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec3 fragTangentWorld;
layout(location = 4) out vec3 fragBitangentWorld;

struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
  vec4 ambientLightColor; // w is intensity
  PointLight pointLights[10];
  ivec4 numLights; // x is the number of active lights
} ubo;

// 128 bytes: model matrix, normal matrix (3 columns), material params
layout(push_constant) uniform Push {
  mat4 modelMatrix;
  vec4 nrmCol0;
  vec4 nrmCol1;
  vec4 nrmCol2;
  vec4 materialParams; // x = metallic, y = roughness, z = ao
} push;

void main() {
  vec4 positionWorld = push.modelMatrix * vec4(Position, 1.0);
  gl_Position = ubo.projection * ubo.view * positionWorld;

  mat3 normalMatrix = mat3(push.nrmCol0.xyz, push.nrmCol1.xyz, push.nrmCol2.xyz);

  // Orthonormal TBN: Gram-Schmidt the tangent against the normal and rebuild
  // the bitangent so the basis stays valid after the model transform.
  vec3 N = normalize(normalMatrix * Normal);
  vec3 T = normalize(normalMatrix * Tangent);
  T = normalize(T - dot(T, N) * N);
  vec3 B = cross(N, T);

  fragPosWorld = positionWorld.xyz;
  fragTexCoord = TexCoords;
  fragNormalWorld = N;
  fragTangentWorld = T;
  fragBitangentWorld = B;
}
