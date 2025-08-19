#include "pbr_lighting_pass.hpp"

namespace voxelvk {

void PBRLightingPass::bind(VkCommandBuffer cmd, VkPipelineLayout layout, VkDescriptorSet set){
    const IBLMaps& maps = resources.getIBLMaps();
    VkDescriptorImageInfo infos[2]{};
    infos[0].sampler = maps.irradianceSampler;
    infos[0].imageView = maps.irradianceView;
    infos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    infos[1].sampler = maps.prefilteredSampler;
    infos[1].imageView = maps.prefilteredView;
    infos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writes[2]{};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = set;
    writes[0].dstBinding = 0;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[0].descriptorCount = 1;
    writes[0].pImageInfo = &infos[0];
    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = set;
    writes[1].dstBinding = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].descriptorCount = 1;
    writes[1].pImageInfo = &infos[1];

    vkUpdateDescriptorSets(device, 2, writes, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &set, 0, nullptr);
}

} // namespace voxelvk
