#include "pbr_pass.hpp"
#include <vector>
#include <cstdio>
#include <cstring>

namespace voxelvk {

static std::vector<char> readFile(const char* path){
    std::vector<char> data;
    FILE* f = std::fopen(path, "rb");
    if(!f) {
        std::fprintf(stderr, "Error: Failed to open file '%s'\n", path);
        return data;
    }
    std::fseek(f, 0, SEEK_END);
    long n = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
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

bool PBRPass::init(VkPhysicalDevice /*phys*/, VkDevice dev, VkFormat colorFormat){
    device = dev;

    VkDescriptorSetLayoutBinding matB[2]{};
    matB[0].binding = 0; matB[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; matB[0].descriptorCount = 1; matB[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    matB[1].binding = 1; matB[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; matB[1].descriptorCount = 1; matB[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo lci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    lci.bindingCount = 2; lci.pBindings = matB;
    if(vkCreateDescriptorSetLayout(device,&lci,nullptr,&materialSetLayout)!=VK_SUCCESS) return false;

    VkDescriptorSetLayoutBinding brdfB{};
    brdfB.binding = 0; brdfB.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; brdfB.descriptorCount = 1; brdfB.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    lci.bindingCount = 1; lci.pBindings = &brdfB;
    if(vkCreateDescriptorSetLayout(device,&lci,nullptr,&brdfSetLayout)!=VK_SUCCESS) return false;

    VkDescriptorSetLayoutBinding shadowB[2]{};
    shadowB[0].binding = 0; shadowB[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; shadowB[0].descriptorCount = 1; shadowB[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    shadowB[1].binding = 1; shadowB[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; shadowB[1].descriptorCount = 1; shadowB[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    lci.bindingCount = 2; lci.pBindings = shadowB;
    if(vkCreateDescriptorSetLayout(device,&lci,nullptr,&shadowSetLayout)!=VK_SUCCESS) return false;

    VkDescriptorSetLayout sets[3] = {materialSetLayout, brdfSetLayout, shadowSetLayout};
    VkPushConstantRange pcr{};
    pcr.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pcr.offset = 0; pcr.size = sizeof(PBRPushConstants);
    VkPipelineLayoutCreateInfo plci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    plci.setLayoutCount = 3; plci.pSetLayouts = sets;
    plci.pushConstantRangeCount = 1; plci.pPushConstantRanges = &pcr;
    if(vkCreatePipelineLayout(device,&plci,nullptr,&pipelineLayout)!=VK_SUCCESS) return false;

    VkShaderModule vs = loadShader(device, "spv/common/fullscreen.vert.spv");
    VkShaderModule fs = loadShader(device, "spv/lighting/pbr_pcss.frag.spv");
    if(!vs || !fs) return false;

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vs; stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fs; stages[1].pName = "main";

    VkPipelineVertexInputStateCreateInfo vi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPipelineViewportStateCreateInfo vp{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    vp.viewportCount = 1; vp.scissorCount = 1;
    VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_NONE;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkPipelineDepthStencilStateCreateInfo ds{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    ds.depthTestEnable = VK_FALSE;
    ds.depthWriteEnable = VK_FALSE;
    VkPipelineColorBlendAttachmentState att{};
    // Additive blending (ONE + ONE) is used here instead of typical alpha blending.
    // This is intentional for [insert reason here, e.g., "accumulating light contributions in a post-processing pass"
    // or "rendering special effects that require additive color blending"]. If this is not the intended effect,
    // consider switching to alpha blending or disabling blending for standard PBR rendering.
    att.blendEnable = VK_TRUE;
    att.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    att.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    att.colorBlendOp = VK_BLEND_OP_ADD;
    att.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    att.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    att.alphaBlendOp = VK_BLEND_OP_ADD;
    att.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    cb.attachmentCount = 1; cb.pAttachments = &att;
    VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dyn.dynamicStateCount = 2; dyn.pDynamicStates = dynStates;

    VkPipelineRenderingCreateInfo ri{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    ri.colorAttachmentCount = 1;
    ri.pColorAttachmentFormats = &colorFormat;

    VkGraphicsPipelineCreateInfo pci{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pci.pNext = &ri;
    pci.stageCount = 2; pci.pStages = stages;
    pci.pVertexInputState = &vi;
    pci.pInputAssemblyState = &ia;
    pci.pViewportState = &vp;
    pci.pRasterizationState = &rs;
    pci.pMultisampleState = &ms;
    pci.pDepthStencilState = &ds;
    pci.pColorBlendState = &cb;
    pci.pDynamicState = &dyn;
    pci.layout = pipelineLayout;

    if(vkCreateGraphicsPipelines(device,nullptr,1,&pci,nullptr,&pipeline)!=VK_SUCCESS) return false;

    vkDestroyShaderModule(device, fs, nullptr);
    vkDestroyShaderModule(device, vs, nullptr);
    return true;
}

void PBRPass::destroy(){
    if(pipeline) vkDestroyPipeline(device, pipeline, nullptr);
    if(pipelineLayout) vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    if(materialSetLayout) vkDestroyDescriptorSetLayout(device, materialSetLayout, nullptr);
    if(brdfSetLayout) vkDestroyDescriptorSetLayout(device, brdfSetLayout, nullptr);
    if(shadowSetLayout) vkDestroyDescriptorSetLayout(device, shadowSetLayout, nullptr);
    device = VK_NULL_HANDLE;
    pipeline = VK_NULL_HANDLE;
    pipelineLayout = VK_NULL_HANDLE;
    materialSetLayout = VK_NULL_HANDLE;
    brdfSetLayout = VK_NULL_HANDLE;
    shadowSetLayout = VK_NULL_HANDLE;
}

void PBRPass::record(VkCommandBuffer cmd,
                     VkDescriptorSet materialSet,
                     VkDescriptorSet brdfSet,
                     VkDescriptorSet shadowSet,
                     const PBRPushConstants& pc){
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    VkDescriptorSet sets[3] = {materialSet, brdfSet, shadowSet};
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 3, sets, 0, nullptr);
    vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PBRPushConstants), &pc);
    vkCmdDraw(cmd, 3, 1, 0, 0);
}

} // namespace voxelvk

