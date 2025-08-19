#pragma once
#include <glm/vec3.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace voxelvk {

struct Material {
    glm::vec3 albedo{0.0f};
    float metallic = 0.0f;
    float roughness = 1.0f;
};

class MaterialManager {
public:
    bool load(const std::string& path = "assets/config/materials.json");
    const Material* material_by_id(int id) const;
    int id_by_name(const std::string& name) const;

private:
    std::vector<Material> materials_;
    std::unordered_map<std::string, int> name_to_id_;
};

} // namespace voxelvk
