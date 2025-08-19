#include "descriptor_pool.hpp"
#include <array>

namespace voxelvk {

bool create_persistent_descriptors(VkDevice device, PersistentDescriptors& out)
{
    // Descriptor set layout bindings
    std::array<VkDescriptorSetLayoutBinding, 7> bindings{};

    // 0 - per frame camera matrices
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    // 1 - material texture array
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 32; // reserve space for many materials
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // 2 - material parameters buffer
    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // 3 - lighting uniform buffer
    bindings[3].binding = 3;
    bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[3].descriptorCount = 1;
    bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // 4 - environment cubemap
    bindings[4].binding = 4;
    bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[4].descriptorCount = 1;
    bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // 5 - cascaded shadow map array
    bindings[5].binding = 5;
    bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[5].descriptorCount = 1;
    bindings[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // 6 - BRDF integration LUT
    bindings[6].binding = 6;
    bindings[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[6].descriptorCount = 1;
    bindings[6].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo lci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    lci.bindingCount = static_cast<uint32_t>(bindings.size());
    lci.pBindings = bindings.data();
    if (vkCreateDescriptorSetLayout(device, &lci, nullptr, &out.layout) != VK_SUCCESS) return false;

    // Descriptor pool sizes
    std::array<VkDescriptorPoolSize, 3> poolSizes{};
    poolSizes[0] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2};
    poolSizes[1] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 35}; // 32 materials + env + shadow + LUT
    poolSizes[2] = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1};

    VkDescriptorPoolCreateInfo pci{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pci.maxSets = 1;
    pci.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    pci.pPoolSizes = poolSizes.data();
    if (vkCreateDescriptorPool(device, &pci, nullptr, &out.pool) != VK_SUCCESS) return false;

    VkDescriptorSetAllocateInfo ai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    ai.descriptorPool = out.pool;
    ai.descriptorSetCount = 1;
    ai.pSetLayouts = &out.layout;
    if (vkAllocateDescriptorSets(device, &ai, &out.set) != VK_SUCCESS) return false;

    return true;
}

void destroy_persistent_descriptors(VkDevice device, PersistentDescriptors& out)
{
    if (out.pool) {
        vkDestroyDescriptorPool(device, out.pool, nullptr);
        out.pool = VK_NULL_HANDLE;
    }
    if (out.layout) {
        vkDestroyDescriptorSetLayout(device, out.layout, nullptr);
        out.layout = VK_NULL_HANDLE;
    }
    out.set = VK_NULL_HANDLE;
}

} // namespace voxelvk

