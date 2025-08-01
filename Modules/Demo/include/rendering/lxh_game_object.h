#pragma once

#include "lxh_model.h"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace lxh {

struct TransformComponent {
  glm::vec3 translation{};
  glm::vec3 scale{1.f, 1.f, 1.f};
  glm::vec3 rotation{};

  // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
  // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
  // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
  glm::mat4 mat4();

  glm::mat3 normalMatrix();
};

struct PointLightComponent {
  float lightIntensity = 1.0f;
};

class LxhGameObject {
 public:
  using id_t = unsigned int;
  using Map = std::unordered_map<id_t, LxhGameObject>;

  static LxhGameObject createGameObject() {
    static id_t currentId = 0;
    return LxhGameObject{currentId++};
  }

  static LxhGameObject makePointLight(
      float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

  LxhGameObject(const LxhGameObject&) = delete;
  LxhGameObject&operator=(const LxhGameObject&) = delete;
  LxhGameObject(LxhGameObject&&) = default;
  LxhGameObject&operator=(LxhGameObject&&) = default;

  id_t getId() { return id; }

  glm::vec3 color{};
  TransformComponent transform{};

  // Optional pointer components
  std::shared_ptr<LxhModel> model{};
  std::unique_ptr<PointLightComponent> pointLight = nullptr;

 private:
     LxhGameObject(id_t objId) : id{objId} {}

  id_t id;
};
}  // namespace lve
