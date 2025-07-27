#pragma once
#include<string>


const char* kCodeComputeTest = R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#ifdef VULKAN
// kBinding_StorageImages in VulkanContext.cpp
layout (set = 3, binding = 6, rgba8) uniform readonly  image2D kTextures2Din[];
layout (set = 3, binding = 6, rgba8) uniform writeonly image2D kTextures2Dout[];

layout(push_constant) uniform PushConstants {
  uint textureId;
} pc;
#else
layout (binding = 2, rgba8) uniform readonly  image2D kTextures2Din;
layout (binding = 2, rgba8) uniform writeonly image2D kTextures2Dout;
#endif

vec4 imageLoad2D(ivec2 uv) {
#ifdef VULKAN
  return imageLoad(kTextures2Din[pc.textureId], uv);
#else
  return imageLoad(kTextures2Din, uv);
#endif
}

void imageStore2D(ivec2 uv, vec4 data) {
#ifdef VULKAN
  imageStore(kTextures2Dout[pc.textureId], uv, data);
#else
  imageStore(kTextures2Dout, uv, data);
#endif
}

void main() {
   vec4 pixel = imageLoad2D(ivec2(gl_GlobalInvocationID.xy));
   float luminance = dot(pixel, vec4(0.299, 0.587, 0.114, 0.0)); // https://www.w3.org/TR/AERT/#color-contrast
   imageStore2D(ivec2(gl_GlobalInvocationID.xy), vec4(vec3(luminance), 1.0));
}
)";

const char* kCodeFullscreenVS = R"(
layout (location=0) out vec2 uv;
void main() {
  // generate a triangle covering the entire screen
  uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
#ifdef VULKAN
  gl_Position = vec4(uv * vec2(2, -2) + vec2(-1, 1), 0.0, 1.0);
#else
  gl_Position = vec4(uv * vec2(2, 2) + vec2(-1, -1), 0.0, 1.0);
#endif
}
)";

const char* kCodeFullscreenFS = R"(
layout (location=0) in vec2 uv;
layout (location=0) out vec4 out_FragColor;

#ifdef VULKAN
layout(set = 0, binding = 0) uniform sampler2D texFullScreen;
#else
uniform sampler2D texFullScreen;
#endif

void main() {
  out_FragColor = texture(texFullScreen, uv);
}
)";

const char* kCodeVS = R"(
layout (location=0) in vec3 pos;
layout (location=1) in vec3 normal;
layout (location=2) in vec2 uv;
#ifdef VULKAN
layout (location=3) in uint mtlIndex;
#else
layout (location=3) in float mtlIndex;
#endif

struct UniformsPerFrame {
  mat4 proj;
  mat4 view;
  mat4 light;
  int bDrawNormals;
  int bDebugLines;
  vec2 padding;
};

struct UniformsPerObject {
  mat4 model;
};

struct Material {
   vec4 ambient;
   vec4 diffuse;
   int texAmbient;
   int texDiffuse;
   int texAlpha;
   int padding;
};

#ifdef VULKAN
layout(set = 1, binding = 0, std140) uniform PerFrame {
  UniformsPerFrame perFrame;
};

layout(set = 1, binding = 1, std140) uniform PerObject {
  UniformsPerObject perObject;
};

layout(set = 1, binding = 2, std430) readonly buffer Materials {
  Material mtl[];
} mat;
#else
uniform PerFrame  {
  UniformsPerFrame perFrame;
};
uniform PerObject {
  UniformsPerObject perObject;
};
uniform MeshMaterials {
  Material materials[132];
};
#endif

// output
struct PerVertex {
  vec3 normal;
  vec2 uv;
  vec4 shadowCoords;
};
layout (location=0) out PerVertex vtx;
layout (location=5) flat out Material mtl;
//

void main() {
  mat4 proj = perFrame.proj;
  mat4 view = perFrame.view;
  mat4 model = perObject.model;
  mat4 light = perFrame.light;
#ifdef VULKAN
  mtl = mat.mtl[uint(mtlIndex)];
#else
  mtl = materials[int(mtlIndex)];
#endif
  gl_Position = proj * view * model * vec4(pos, 1.0);

  // Compute the normal in world-space
  mat3 norm_matrix = transpose(inverse(mat3(model)));
  vtx.normal = normalize(norm_matrix * normal);
  vtx.uv = uv;
  vtx.shadowCoords = light * model * vec4(pos, 1.0);
}
)";

const char* kCodeVS_Wireframe = R"(
layout (location=0) in vec3 pos;

struct UniformsPerFrame {
  mat4 proj;
  mat4 view;
};

struct UniformsPerObject {
  mat4 model;
};

#ifdef VULKAN
layout(set = 1, binding = 0, std140)
#endif
uniform PerFrame {
  UniformsPerFrame perFrame;
};
#ifdef VULKAN
layout(set = 1, binding = 1, std140)
#endif
uniform PerObject{
  UniformsPerObject perObject;
};

void main() {
  mat4 proj = perFrame.proj;
  mat4 view = perFrame.view;
  mat4 model = perObject.model;
  gl_Position = proj * view * model * vec4(pos, 1.0);
}
)";

const char* kCodeFS_Wireframe = R"(
layout (location=0) out vec4 out_FragColor;

void main() {
  out_FragColor = vec4(1.0);
};
)";

const char* kCodeFS = R"(
struct UniformsPerFrame {
  mat4 proj;
  mat4 view;
  mat4 light;
  int bDrawNormals;
  int bDebugLines;
  vec2 padding;
};
#ifdef VULKAN
layout(set = 1, binding = 0, std140)
#endif
uniform PerFrame {
  UniformsPerFrame perFrame;
};

struct Material {
  vec4 ambient;
  vec4 diffuse;
  int texAmbient;
  int texDiffuse;
  int texAlpha;
  int padding;
};
struct PerVertex {
  vec3 normal;
  vec2 uv;
  vec4 shadowCoords;
};

layout (location=0) in PerVertex vtx;
layout (location=5) flat in Material mtl;

layout (location=0) out vec4 out_FragColor;

#ifdef VULKAN
layout(set = 0, binding = 0) uniform sampler2DShadow texShadow;
layout(set = 0, binding = 4) uniform samplerCube texSkyboxIrradiance;

vec4 textureBindless2D(uint textureid, vec2 uv) {
  return texture(sampler2D(kTextures2D[textureid], kSamplers[1]), uv);
}
#else
  layout(binding = 0) uniform sampler2D texShadow;
  layout(binding = 1) uniform sampler2D texAmbient;
  layout(binding = 2) uniform sampler2D texDiffuse;
  layout(binding = 3) uniform sampler2D texAlpha;
  layout(binding = 4) uniform samplerCube texSkyboxIrradiance;
#endif // VULKAN

float PCF3(vec3 uvw) {
  float size = 1.0 / float( textureSize(texShadow, 0).x );
  float shadow = 0.0;
  for (int v=-1; v<=+1; v++)
    for (int u=-1; u<=+1; u++)
#ifdef VULKAN
      shadow += texture(texShadow, uvw + size * vec3(u, v, 0));
#else
      shadow += (uvw.z <= texture(texShadow, uvw.xy + size * vec2(u, v) ).r) ? 1.0 : 0.0;
#endif
  return shadow / 9;
}

float shadow(vec4 s) {
  s = s / s.w;
  if (s.z > -1.0 && s.z < 1.0) {
    float depthBias = -0.00005;
#ifdef VULKAN
    s.y = 1.0 - s.y;
#endif
    float shadowSample = PCF3(vec3(s.x, s.y, s.z + depthBias));
    return mix(0.3, 1.0, shadowSample);
  }
  return 1.0;
}

void main() {
#ifdef VULKAN
  vec4 alpha = textureBindless2D(mtl.texAlpha, vtx.uv);
  if (mtl.texAlpha > 0 && alpha.r < 0.5)
    discard;
  vec4 Ka = mtl.ambient * textureBindless2D(mtl.texAmbient, vtx.uv);
  vec4 Kd = mtl.diffuse * textureBindless2D(mtl.texDiffuse, vtx.uv);
#else
  vec4 alpha = texture(texAlpha, vtx.uv);
  // check it is not a dummy 1x1 texture
  if (textureSize(texAlpha, 0).x > 1 && alpha.r < 0.5)
    discard;
  vec4 Ka = mtl.ambient * texture(texAmbient, vtx.uv);
  vec4 Kd = mtl.diffuse * texture(texDiffuse, vtx.uv);
#endif
  bool drawNormals = perFrame.bDrawNormals > 0;
  if (Kd.a < 0.5)
    discard;
  vec3 n = normalize(vtx.normal);
  float NdotL1 = clamp(dot(n, normalize(vec3(-1, 1,+1))), 0.0, 1.0);
  float NdotL2 = clamp(dot(n, normalize(vec3(-1, 1,-1))), 0.0, 1.0);
  float NdotL = 0.5 * (NdotL1+NdotL2);
  // IBL diffuse
  const vec4 f0 = vec4(0.04);
  vec4 diffuse = texture(texSkyboxIrradiance, n) * Kd * (vec4(1.0) - f0);
  out_FragColor = drawNormals ?
    vec4(0.5 * (n+vec3(1.0)), 1.0) :
    Ka + diffuse * shadow(vtx.shadowCoords);
};
)";

const char* kShadowVS = R"(
layout (location=0) in vec3 pos;

struct UniformsPerFrame {
  mat4 proj;
  mat4 view;
  mat4 light;
  int bDrawNormals;
  int bDebugLines;
  vec2 padding;
};

struct UniformsPerObject {
  mat4 model;
};

#ifdef VULKAN
layout(set = 1, binding = 0, std140) uniform PerFrame {
  UniformsPerFrame perFrame;
};
layout(set = 1, binding = 1, std140) uniform PerObject {
  UniformsPerObject perObject;
};
#else
uniform ShadowFrameUniforms {
   UniformsPerFrame perFrame;
};
uniform ShadowObjectUniforms {
  UniformsPerObject perObject;
};

#endif
void main() {
  mat4 proj = perFrame.proj;
  mat4 view = perFrame.view;
  mat4 model = perObject.model;
  gl_Position = proj * view * model * vec4(pos, 1.0);
}
)";

const char* kShadowFS = R"(
void main() {
};
)";

const char* kSkyboxVS = R"(
layout (location=0) out vec3 textureCoords;

const vec3 positions[8] = vec3[8](
	vec3(-1.0,-1.0, 1.0), vec3( 1.0,-1.0, 1.0), vec3( 1.0, 1.0, 1.0), vec3(-1.0, 1.0, 1.0),
	vec3(-1.0,-1.0,-1.0), vec3( 1.0,-1.0,-1.0), vec3( 1.0, 1.0,-1.0), vec3(-1.0, 1.0,-1.0)
);

const int indices[36] = int[36](
	0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1, 7, 6, 5, 5, 4, 7, 4, 0, 3, 3, 7, 4, 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3
);

struct UniformsPerFrame {
  mat4 proj;
  mat4 view;
  mat4 light;
  int bDrawNormals;
  int bDebugLines;
  vec2 padding;
};

#ifdef VULKAN
layout(set = 1, binding = 0, std140) uniform PerFrame {
  UniformsPerFrame perFrame;
};
#else
uniform SkyboxFrameUniforms {
  UniformsPerFrame perFrame;
};

#endif
void main() {
  mat4 proj = perFrame.proj;
  mat4 view = perFrame.view;
  // discard translation
  view = mat4(view[0], view[1], view[2], vec4(0, 0, 0, 1));
  mat4 transform = proj * view;
  vec3 pos = positions[indices[gl_VertexIndex]];
  gl_Position = (transform * vec4(pos, 1.0)).xyww;

  // skybox
  textureCoords = pos;
#ifdef VULKAN
  // Draws the skybox edges. One color per edge
  const bool drawDebugLines = perFrame.bDebugLines > 0;
  if (drawDebugLines) {
      const int[12][2] edgeIndices = {
          {0,1}, {1,2}, {2,3}, {3,0}, {4,5}, {5,6}, {6,7}, {7,4}, {0,4}, {1,5}, {2,6}, {3,7}
      };

      const vec4 edgeColors[12] = vec4[12](
        vec4(  1,   0,   0, 1), vec4(  1,   1,   0, 1), vec4(  0,   1,   0, 1), vec4(  0,   1, 1, 1),
        vec4(  1,   0,   1, 1), vec4(  0,   0,   1, 1), vec4(  1,   1,   1, 1), vec4(  0,   0, 0, 1),
        vec4(0.5, 0.7, 0.8, 1), vec4(0.4, 0.4, 0.4, 1), vec4(  1, 0.3, 0.6, 1), vec4(  1, 0.8, 0, 1)
      );

      uint index = gl_VertexIndex / 3;
      drawLine(positions[edgeIndices[index][0]],
                positions[edgeIndices[index][1]],
                edgeColors[index],
                edgeColors[index],
                transform);
  }
#endif
}

)";
const char* kSkyboxFS = R"(
layout (location=0) in vec3 textureCoords;
layout (location=0) out vec4 out_FragColor;

#ifdef VULKAN
layout(set = 0, binding = 1) uniform samplerCube texSkybox;
#else
uniform samplerCube texSkybox;
#endif

void main() {
  out_FragColor = texture(texSkybox, textureCoords);
}
)";