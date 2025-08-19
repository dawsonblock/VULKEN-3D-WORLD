#include "pbr_lighting_pass.hpp"
#include "csm_pass.hpp"
#include <vector>
#include <cstdio>
#include <cstring>

namespace voxelvk {

static std::vector<char> readFile(const char* path){
    std::vector<char> data;
    FILE* f = std::fopen(path, "rb");
    if(!f) return data;
    std::fseek(f,0,SEEK_END); long n=std::ftell(f); std::fseek(f,0,SEEK_SET);
    data.resize(n);
    std::fread(data.data(),1,n,f);
    std::fclose(f);
    return data;
}

static VkShaderModule loadShader(VkDevice dev, const char* path){
    auto bytes = readFile(path);
    if(bytes.empty()) return VK_NULL_HANDLE;
    VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    ci.codeSize = bytes.size();
    ci.pCode = reinterpret_cast<const uint32_t*>(bytes.data());
    VkShaderModule m;
    if(vkCreateShaderModule(dev,&ci,nullptr,&m)!=VK_SUCCESS) return VK_NULL_HANDLE;
    return m;
}

bool PBRLightingPass::init(VkPhysicalDevice /*phys*/, VkDevice dev, VmaAllocator alloc,
                           VkFormat colorFormat, VkDescriptorSetLayout csmSetLayout){
    device = dev; allocator = alloc;

    VkDescriptorSetLayoutBinding b[3]{};
    b[0].binding = 0; b[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; b[0].descriptorCount = 1; b[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    b[1].binding = 1; b[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; b[1].descriptorCount = 1; b[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    b[2].binding = 5; b[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; b[2].descriptorCount = 1; b[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo lci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    lci.bindingCount = 3; lci.pBindings = b;
    if(vkCreateDescriptorSetLayout(device,&lci,nullptr,&setLayout)!=VK_SUCCESS) return false;

    VkDescriptorPoolSize ps{}; ps.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; ps.descriptorCount = 3;
    VkDescriptorPoolCreateInfo pci{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pci.maxSets = 1; pci.poolSizeCount = 1; pci.pPoolSizes = &ps;
    if(vkCreateDescriptorPool(device,&pci,nullptr,&descPool)!=VK_SUCCESS) return false;

    VkDescriptorSetAllocateInfo ai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    ai.descriptorPool = descPool; ai.descriptorSetCount = 1; ai.pSetLayouts = &setLayout;
    if(vkAllocateDescriptorSets(device,&ai,&descSet)!=VK_SUCCESS) return false;

    VkPushConstantRange pcr{}; pcr.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; pcr.offset = 0; pcr.size = sizeof(PBRPushConstants);
    VkDescriptorSetLayout sets[2] = { setLayout, csmSetLayout };
    uint32_t setCount = csmSetLayout != VK_NULL_HANDLE ? 2u : 1u;
    VkPipelineLayoutCreateInfo plci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    plci.setLayoutCount = setCount; plci.pSetLayouts = sets; plci.pushConstantRangeCount = 1; plci.pPushConstantRanges = &pcr;
    if(vkCreatePipelineLayout(device,&plci,nullptr,&pipelineLayout)!=VK_SUCCESS) return false;

    VkShaderModule vs = loadShader(device, "spv/pbr_lighting.vert.spv");
    VkShaderModule fs = loadShader(device, "spv/pbr_lighting.frag.spv");
    if(!vs || !fs) return false;

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT; stages[0].module = vs; stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; stages[1].module = fs; stages[1].pName = "main";

    VkVertexInputBindingDescription bind{}; bind.binding = 0; bind.stride = sizeof(float)*8; bind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    VkVertexInputAttributeDescription attr[3]{};
    attr[0].location=0; attr[0].binding=0; attr[0].format=VK_FORMAT_R32G32B32_SFLOAT; attr[0].offset=0;
    attr[1].location=1; attr[1].binding=0; attr[1].format=VK_FORMAT_R32G32B32_SFLOAT; attr[1].offset=sizeof(float)*3;
    attr[2].location=2; attr[2].binding=0; attr[2].format=VK_FORMAT_R32G32_SFLOAT; attr[2].offset=sizeof(float)*6;
    VkPipelineVertexInputStateCreateInfo vi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vi.vertexBindingDescriptionCount = 1; vi.pVertexBindingDescriptions = &bind; vi.vertexAttributeDescriptionCount = 3; vi.pVertexAttributeDescriptions = attr;

    VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo vp{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    vp.viewportCount = 1; vp.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rs.polygonMode = VK_POLYGON_MODE_FILL; rs.cullMode = VK_CULL_MODE_BACK_BIT; rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo ds{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    ds.depthTestEnable = VK_TRUE; ds.depthWriteEnable = VK_TRUE; ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkPipelineColorBlendAttachmentState att{};
    att.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    cb.attachmentCount = 1; cb.pAttachments = &att;

    VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dyn.dynamicStateCount = 2; dyn.pDynamicStates = dynStates;

    VkPipelineRenderingCreateInfo ri{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    ri.colorAttachmentCount = 1; ri.pColorAttachmentFormats = &colorFormat; ri.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

    VkGraphicsPipelineCreateInfo pci{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pci.pNext = &ri; pci.stageCount = 2; pci.pStages = stages; pci.pVertexInputState = &vi; pci.pInputAssemblyState = &ia;
    pci.pViewportState = &vp; pci.pRasterizationState = &rs; pci.pMultisampleState = &ms; pci.pDepthStencilState = &ds; pci.pColorBlendState = &cb; pci.pDynamicState = &dyn; pci.layout = pipelineLayout;
    if(vkCreateGraphicsPipelines(device,nullptr,1,&pci,nullptr,&pipeline)!=VK_SUCCESS) return false;

    vkDestroyShaderModule(device, fs, nullptr);
    vkDestroyShaderModule(device, vs, nullptr);
    return true;
}

void PBRLightingPass::destroy(){
    if(pipeline) vkDestroyPipeline(device, pipeline, nullptr);
    if(pipelineLayout) vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    if(descPool) vkDestroyDescriptorPool(device, descPool, nullptr);
    if(setLayout) vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
    device = VK_NULL_HANDLE;
}

void PBRLightingPass::updateDescriptors(VkImageView albedo, VkSampler albedoSampler,
                                        VkImageView metalRough, VkSampler metalRoughSampler,
                                        VkImageView shadowArray, VkSampler shadowSampler){
    VkDescriptorImageInfo ii[3]{};
    ii[0].imageView = albedo; ii[0].sampler = albedoSampler; ii[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ii[1].imageView = metalRough; ii[1].sampler = metalRoughSampler; ii[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ii[2].imageView = shadowArray; ii[2].sampler = shadowSampler; ii[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet w[3]{};
    for(int i=0;i<3;i++){
        w[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w[i].dstSet = descSet;
        w[i].dstBinding = (i==2)?5:i;
        w[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        w[i].descriptorCount = 1;
        w[i].pImageInfo = &ii[i];
    }
    vkUpdateDescriptorSets(device, 3, w, 0, nullptr);
}

void PBRLightingPass::record(VkCommandBuffer cmd, VkDescriptorSet csmSet,
                             const PBRPushConstants& pc, uint32_t indexCount) const{
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    VkDescriptorSet sets[2] = { descSet, csmSet };
    uint32_t count = csmSet != VK_NULL_HANDLE ? 2u : 1u;
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, count, sets, 0, nullptr);
    vkCmdPushConstants(cmd, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(PBRPushConstants), &pc);
    vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
}

} // namespace voxelvk
