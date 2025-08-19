#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

struct VmaAllocator_T;
using VmaAllocator = VmaAllocator_T*;
struct VmaAllocation_T;
using VmaAllocation = VmaAllocation_T*;

namespace voxelvk {

struct MaterialGPU {
    glm::vec4 albedo;       // rgb + metallic in w
    glm::vec4 params;       // roughness in x
};

struct MaterialRecord {
    glm::vec3    albedo{0.0f};
    float        metallic{0.0f};
    float        roughness{1.0f};
    VkBuffer     ubo{VK_NULL_HANDLE};
    VmaAllocation alloc{nullptr};
    VkDescriptorSet descSet{VK_NULL_HANDLE};
};

class MaterialManager {
public:
    bool init(VkDevice dev, VmaAllocator alloc, const std::string& jsonPath = "assets/config/materials.json");
    void destroy();

    const MaterialRecord* get(const std::string& name) const;
    void bind(VkCommandBuffer cmd, VkPipelineLayout layout, uint32_t setIndex, const std::string& name) const;

    VkDescriptorSetLayout getSetLayout() const { return setLayout_; }

private:
    VkDevice device_{VK_NULL_HANDLE};
    VmaAllocator allocator_{nullptr};
    VkDescriptorSetLayout setLayout_{VK_NULL_HANDLE};
    VkDescriptorPool descriptorPool_{VK_NULL_HANDLE};
    std::unordered_map<std::string, MaterialRecord> materials_;
};

} // namespace voxelvk

