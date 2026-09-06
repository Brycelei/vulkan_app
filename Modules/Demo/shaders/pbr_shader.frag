#version 450

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec3 fragTangentWorld;
layout(location = 4) in vec3 fragBitangentWorld;

layout(location = 0) out vec4 outColor;

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

// set 1: PBR material textures (combined image samplers)
layout(set = 1, binding = 0) uniform sampler2D albedoSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;
layout(set = 1, binding = 2) uniform sampler2D roughnessSampler;
layout(set = 1, binding = 3) uniform sampler2D aoSampler;

// 128 bytes: model matrix, normal matrix (3 columns), material params
layout(push_constant) uniform Push {
  mat4 modelMatrix;
  vec4 nrmCol0;
  vec4 nrmCol1;
  vec4 nrmCol2;
  vec4 materialParams; // x = metallic, y = roughness, z = ao
} push;

const float PI = 3.14159265359;

// GGX / Trowbridge-Reitz normal distribution
float distributionGGX(float NdotH, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH2 = NdotH * NdotH;
  float num = a2;
  float denom = NdotH2 * (a2 - 1.0) + 1.0;
  denom = PI * denom * denom;
  return num / max(denom, 1e-5);
}

// Smith geometry term with the Schlick-GGX approximation
float geometrySchlickGGX(float NdotV, float roughness) {
  float r = roughness + 1.0;
  float k = (r * r) / 8.0;
  float num = NdotV;
  float denom = NdotV * (1.0 - k) + k;
  return num / max(denom, 1e-5);
}

float geometrySmith(float NdotV, float NdotL, float roughness) {
  return geometrySchlickGGX(NdotV, roughness) * geometrySchlickGGX(NdotL, roughness);
}

// Schlick's approximation of the Fresnel factor
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(clamp(cosTheta, 0.0, 1.0), 5.0);
}

void main() {
  // glTF metallic-roughness convention: roughness factor scales the .g channel
  // of the roughness map, AO factor scales the .r channel of the AO map.
  vec3 albedo = texture(albedoSampler, fragTexCoord).rgb;
  float metallic = push.materialParams.x;
  float roughness = clamp(texture(roughnessSampler, fragTexCoord).g * push.materialParams.y, 0.045, 1.0);
  float ao = texture(aoSampler, fragTexCoord).r * push.materialParams.z;

  // Perturb the shading normal with the tangent-space normal map
  vec3 normalSample = texture(normalSampler, fragTexCoord).rgb * 2.0 - 1.0;
  vec3 N = normalize(mat3(fragTangentWorld, fragBitangentWorld, fragNormalWorld) * normalSample);

  // invView's translation column is the camera position in world space
  vec3 V = normalize(ubo.invView[3].xyz - fragPosWorld);

  // Dielectrics use a constant F0 of 0.04, metals are tinted by the albedo
  vec3 F0 = mix(vec3(0.04), albedo, metallic);

  vec3 Lo = vec3(0.0);
  for (int i = 0; i < ubo.numLights.x; ++i) {
    vec3 L = ubo.pointLights[i].position.xyz - fragPosWorld;
    float distance = length(L);
    L /= max(distance, 1e-4);

    // Inverse-square attenuation: radiance falls off with squared distance
    vec3 radiance = ubo.pointLights[i].color.rgb * ubo.pointLights[i].color.w
      / max(distance * distance, 1e-4);

    vec3 H = normalize(V + L);

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    // Cook-Torrance specular BRDF
    float D = distributionGGX(max(dot(N, H), 0.0), roughness);
    float G = geometrySmith(NdotV, NdotL, roughness);
    vec3  F = fresnelSchlick(HdotV, F0);

    vec3 specular = (D * G * F) / max(4.0 * NdotV * NdotL, 1e-4);

    // Energy conservation: diffuse shrinks as specular (Fresnel) grows,
    // and metals have (almost) no diffuse term
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);

    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
  }

  // Ambient contribution scaled by the ambient occlusion map
  vec3 ambient = ubo.ambientLightColor.rgb * ubo.ambientLightColor.w * albedo * ao;

  vec3 color = ambient + Lo;

  // Reinhard tonemapping; the sRGB conversion is performed by the
  // swapchain's _SRGB image format
  color = color / (color + vec3(1.0));

  outColor = vec4(color, 1.0);
}
