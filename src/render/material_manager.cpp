#include "material_manager.hpp"
#include <fstream>
#include <regex>
#include <sstream>
#include <iterator>
#include <cstring>

namespace voxelvk {

bool MaterialManager::load(const char* jsonPath) {
    std::ifstream in(jsonPath);
    if (!in) return false;
    std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    std::regex matR("\"([A-Za-z0-9_]+)\"\\s*:\\s*\{([^}]*)\}");
    auto begin = std::sregex_iterator(text.begin(), text.end(), matR);
    auto end = std::sregex_iterator();

    materials_.clear();
    nameToIndex_.clear();

    for (auto it = begin; it != end; ++it) {
        std::string name = (*it)[1].str();
        std::string body = (*it)[2].str();
        Material m{};
        std::smatch sm;
        std::regex albR("\"albedo\"\\s*:\\s*\\[([^\\]]+)\\]");
        if (std::regex_search(body, sm, albR)) {
            std::stringstream ss(sm[1].str());
            char comma;
            for (int i = 0; i < 3; ++i) {
                ss >> m.albedo[i];
                if (i < 2) ss >> comma; // consume comma
            }
        }
        std::regex metR("\"metallic\"\\s*:\\s*([0-9eE+\-.]+)");
        if (std::regex_search(body, sm, metR)) {
            m.metallic = std::stof(sm[1]);
        }
        std::regex rouR("\"roughness\"\\s*:\\s*([0-9eE+\-.]+)");
        if (std::regex_search(body, sm, rouR)) {
            m.roughness = std::stof(sm[1]);
        }
        nameToIndex_[name] = static_cast<uint32_t>(materials_.size());
        materials_.push_back(m);
    }
    return !materials_.empty();
}

uint32_t MaterialManager::findMemoryType(VkPhysicalDevice phys, uint32_t typeBits, VkMemoryPropertyFlags flags) {
    VkPhysicalDeviceMemoryProperties props{};
    vkGetPhysicalDeviceMemoryProperties(phys, &props);
    for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
        if ((typeBits & (1u << i)) && (props.memoryTypes[i].propertyFlags & flags) == flags)
            return i;
    }
    return UINT32_MAX;
}

bool MaterialManager::upload(VkPhysicalDevice phys, VkDevice device) {
    if (materials_.empty()) return false;
    VkDeviceSize size = materials_.size() * sizeof(Material);
    VkBufferCreateInfo bci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bci.size = size;
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (vkCreateBuffer(device, &bci, nullptr, &buffer_) != VK_SUCCESS) return false;
    VkMemoryRequirements req{};
    vkGetBufferMemoryRequirements(device, buffer_, &req);
    uint32_t type = findMemoryType(phys, req.memoryTypeBits,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (type == UINT32_MAX) return false;
    VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    mai.allocationSize = req.size;
    mai.memoryTypeIndex = type;
    if (vkAllocateMemory(device, &mai, nullptr, &memory_) != VK_SUCCESS) return false;
    vkBindBufferMemory(device, buffer_, memory_, 0);
    void* ptr = nullptr;
    vkMapMemory(device, memory_, 0, size, 0, &ptr);
    std::memcpy(ptr, materials_.data(), static_cast<size_t>(size));
    vkUnmapMemory(device, memory_);
    return true;
}

VkDescriptorBufferInfo MaterialManager::descriptorInfo() const {
    VkDescriptorBufferInfo info{};
    info.buffer = buffer_;
    info.offset = 0;
    info.range = materials_.size() * sizeof(Material);
    return info;
}

void MaterialManager::destroy(VkDevice device) {
    if (buffer_) vkDestroyBuffer(device, buffer_, nullptr);
    if (memory_) vkFreeMemory(device, memory_, nullptr);
    buffer_ = VK_NULL_HANDLE;
    memory_ = VK_NULL_HANDLE;
    materials_.clear();
    nameToIndex_.clear();
}

} // namespace voxelvk
