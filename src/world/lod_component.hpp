#pragma once
#include <string>
#include <glm/vec3.hpp>

namespace voxelvk {

enum class LODLevel { High = 0, Medium = 1, Low = 2 };

struct LODConfig {
    float high_detail = 20.0f;
    float medium_detail = 60.0f;
};

class LODComponent {
public:
    LODComponent() = default;
    explicit LODComponent(const std::string& cfgPath);

    void LoadConfig(const std::string& cfgPath);
    LODLevel Select(float distance) const;
    LODLevel Select(const glm::vec3& cameraPos, const glm::vec3& objectPos) const;
    const LODConfig& Config() const { return cfg_; }

private:
    LODConfig cfg_{};
};

} // namespace voxelvk
