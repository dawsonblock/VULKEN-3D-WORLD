#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <string>

namespace voxelvk {

struct Material {
    glm::vec3 albedo{1.0f, 1.0f, 1.0f};
    float metallic = 0.0f;
    float roughness = 1.0f;
};

class MaterialManager {
public:
    // Parse materials from a JSON file.
    bool load(const char* jsonPath);

    // Upload parsed materials to a uniform buffer for GPU access.
    bool upload(VkPhysicalDevice phys, VkDevice device);

    // Release GPU resources.
    void destroy(VkDevice device);

    // Descriptor information for binding in descriptor sets.
    VkDescriptorBufferInfo descriptorInfo() const;

    // Number of materials loaded.
    size_t count() const { return materials_.size(); }

private:
    static uint32_t findMemoryType(VkPhysicalDevice phys, uint32_t typeBits, VkMemoryPropertyFlags flags);

    std::unordered_map<std::string, uint32_t> nameToIndex_;
    std::vector<Material> materials_;
    VkBuffer buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory memory_ = VK_NULL_HANDLE;
};

} // namespace voxelvk
