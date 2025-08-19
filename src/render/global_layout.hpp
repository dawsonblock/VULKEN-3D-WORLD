#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "csm_layout.hpp"

namespace voxelvk {

// Descriptor binding for a 2D BRDF LUT texture.
inline VkDescriptorSetLayoutBinding brdf_lut_binding(uint32_t binding = 6,
                                                     VkShaderStageFlags stages = VK_SHADER_STAGE_FRAGMENT_BIT){
    VkDescriptorSetLayoutBinding b{};
    b.binding = binding;
    b.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    b.descriptorCount = 1;
    b.stageFlags = stages;
    b.pImmutableSamplers = nullptr;
    return b;
}

// Generic helper for cube-map samplers (irradiance/prefiltered environment).
inline VkDescriptorSetLayoutBinding cubemap_binding(uint32_t binding,
                                                    VkShaderStageFlags stages = VK_SHADER_STAGE_FRAGMENT_BIT){
    VkDescriptorSetLayoutBinding b{};
    b.binding = binding;
    b.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    b.descriptorCount = 1;
    b.stageFlags = stages;
    b.pImmutableSamplers = nullptr;
    return b;
}

// Builds a descriptor set layout for the global lighting resources.
// Layout:
//  binding 3 -> CSMData UBO
//  binding 4 -> irradiance samplerCube
//  binding 5 -> shadow map sampler2DArray
//  binding 6 -> BRDF LUT sampler2D
//  binding 7 -> prefiltered environment samplerCube
inline bool create_global_descriptor_layout(VkDevice device, VkDescriptorSetLayout& out){
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    VkDescriptorSetLayoutBinding csmUbo{};
    csmUbo.binding = 3;
    csmUbo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    csmUbo.descriptorCount = 1;
    csmUbo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings.push_back(csmUbo);

    bindings.push_back(cubemap_binding(4));
    bindings.push_back(csm_shadow_binding(5));
    bindings.push_back(brdf_lut_binding(6));
    bindings.push_back(cubemap_binding(7));

    VkDescriptorSetLayoutCreateInfo ci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    ci.bindingCount = static_cast<uint32_t>(bindings.size());
    ci.pBindings = bindings.data();
    return vkCreateDescriptorSetLayout(device, &ci, nullptr, &out) == VK_SUCCESS;
}

// Updates a global descriptor set with the provided resources.
inline void update_global_descriptor_set(VkDevice device, VkDescriptorSet set,
                                         VkBuffer csmUbo, VkDeviceSize csmSize,
                                         VkImageView irradianceView, VkSampler irradianceSampler,
                                         VkImageView shadowView, VkSampler shadowSampler,
                                         VkImageView brdfView, VkSampler brdfSampler,
                                         VkImageView prefilteredView, VkSampler prefilteredSampler){
    VkDescriptorBufferInfo bufInfo{};
    bufInfo.buffer = csmUbo;
    bufInfo.offset = 0;
    bufInfo.range  = csmSize;

    VkDescriptorImageInfo irrInfo{}; irrInfo.sampler = irradianceSampler; irrInfo.imageView = irradianceView; irrInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkDescriptorImageInfo shdInfo{}; shdInfo.sampler = shadowSampler; shdInfo.imageView = shadowView; shdInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkDescriptorImageInfo brdfInfo{}; brdfInfo.sampler = brdfSampler; brdfInfo.imageView = brdfView; brdfInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkDescriptorImageInfo preInfo{}; preInfo.sampler = prefilteredSampler; preInfo.imageView = prefilteredView; preInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    std::vector<VkWriteDescriptorSet> writes(5);

    writes[0] = VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    writes[0].dstSet = set; writes[0].dstBinding = 3; writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; writes[0].pBufferInfo = &bufInfo;

    writes[1] = VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    writes[1].dstSet = set; writes[1].dstBinding = 4; writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; writes[1].pImageInfo = &irrInfo;

    writes[2] = VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    writes[2].dstSet = set; writes[2].dstBinding = 5; writes[2].descriptorCount = 1;
    writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; writes[2].pImageInfo = &shdInfo;

    writes[3] = VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    writes[3].dstSet = set; writes[3].dstBinding = 6; writes[3].descriptorCount = 1;
    writes[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; writes[3].pImageInfo = &brdfInfo;

    writes[4] = VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    writes[4].dstSet = set; writes[4].dstBinding = 7; writes[4].descriptorCount = 1;
    writes[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; writes[4].pImageInfo = &preInfo;

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

} // namespace voxelvk
