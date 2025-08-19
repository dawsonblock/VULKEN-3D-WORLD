#include "material_manager.hpp"
#include <fstream>
#include <sstream>

namespace voxelvk {

bool MaterialManager::load(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) return false;
    std::string json((std::istreambuf_iterator<char>(ifs)), {});
    size_t pos = json.find("\"materials\"");
    if (pos == std::string::npos) return false;
    pos = json.find('{', pos);
    while (pos != std::string::npos) {
        size_t name_start = json.find('"', pos + 1);
        if (name_start == std::string::npos) break;
        size_t name_end = json.find('"', name_start + 1);
        if (name_end == std::string::npos) break;
        std::string name = json.substr(name_start + 1, name_end - name_start - 1);
        size_t albedo_start = json.find('[', name_end);
        size_t albedo_end = json.find(']', albedo_start);
        if (albedo_start == std::string::npos || albedo_end == std::string::npos) break;
        std::string albedo_str = json.substr(albedo_start + 1, albedo_end - albedo_start - 1);
        std::stringstream ss(albedo_str);
        float r, g, b;
        char comma;
        ss >> r >> comma >> g >> comma >> b;
        size_t metallic_pos = json.find("metallic", albedo_end);
        metallic_pos = json.find(':', metallic_pos);
        size_t metallic_end = json.find(',', metallic_pos);
        float metallic = std::stof(json.substr(metallic_pos + 1, metallic_end - metallic_pos - 1));
        size_t roughness_pos = json.find("roughness", metallic_end);
        roughness_pos = json.find(':', roughness_pos);
        size_t mat_end = json.find('}', roughness_pos);
        float roughness = std::stof(json.substr(roughness_pos + 1, mat_end - roughness_pos - 1));
        int id = static_cast<int>(materials_.size());
        materials_.push_back({glm::vec3(r, g, b), metallic, roughness});
        name_to_id_[name] = id;
        pos = json.find('"', mat_end + 1);
    }
    return !materials_.empty();
}

const Material* MaterialManager::material_by_id(int id) const {
    if (id < 0 || static_cast<size_t>(id) >= materials_.size()) return nullptr;
    return &materials_[id];
}

int MaterialManager::id_by_name(const std::string& name) const {
    auto it = name_to_id_.find(name);
    if (it == name_to_id_.end()) return -1;
    return it->second;
}

} // namespace voxelvk
