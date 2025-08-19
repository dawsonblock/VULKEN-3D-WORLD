#pragma once
#include <vulkan/vulkan.h>

namespace voxelvk {

struct IBLMaps {
    VkImageView irradianceView = VK_NULL_HANDLE;
    VkSampler   irradianceSampler = VK_NULL_HANDLE;
    VkImageView prefilteredView = VK_NULL_HANDLE;
    VkSampler   prefilteredSampler = VK_NULL_HANDLE;
};

class ResourceManager {
public:
    void setIBLMaps(const IBLMaps& maps) { iblMaps = maps; }
    const IBLMaps& getIBLMaps() const { return iblMaps; }
private:
    IBLMaps iblMaps{};
};

} // namespace voxelvk
