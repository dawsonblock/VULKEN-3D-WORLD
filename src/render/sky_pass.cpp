#include "sky_pass.hpp"
#include <vector>
#include <cstdio>
#include <cstring>

namespace voxelvk {

static std::vector<char> readFile(const char* path){
    std::vector<char> data;
    FILE* f = std::fopen(path, "rb");
    if(!f) {
        std::fprintf(stderr, "Error: Failed to open file '%s' for reading in readFile.\n", path);
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

bool SkyPass::init(VkPhysicalDevice /*phys*/, VkDevice dev, VkFormat colorFormat){
    device = dev;

    VkDescriptorSetLayoutBinding b{};
    b.binding = 0;
    b.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    b.descriptorCount = 1;
    b.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo lci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    lci.bindingCount = 1; lci.pBindings = &b;
    if(vkCreateDescriptorSetLayout(device,&lci,nullptr,&noiseSetLayout)!=VK_SUCCESS) return false;

    VkPushConstantRange pcr{};
    pcr.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pcr.offset = 0;
    pcr.size = sizeof(SkyPushConstants);
    VkPipelineLayoutCreateInfo plci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    plci.setLayoutCount = 1; plci.pSetLayouts = &noiseSetLayout;
    plci.pushConstantRangeCount = 1; plci.pPushConstantRanges = &pcr;
    if(vkCreatePipelineLayout(device,&plci,nullptr,&pipelineLayout)!=VK_SUCCESS) return false;

    VkShaderModule vs = loadShader(device, "spv/common/fullscreen.vert.spv");
    VkShaderModule fsSky = loadShader(device, "spv/post/atmosphere.frag.spv");
    VkShaderModule fsCloud = loadShader(device, "spv/post/clouds.frag.spv");
    if(!vs || !fsSky || !fsCloud) return false;

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vs; stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].pName = "main";

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

    // Sky pipeline
    stages[1].module = fsSky;
    att.blendEnable = VK_FALSE;
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
    if(vkCreateGraphicsPipelines(device,nullptr,1,&pci,nullptr,&skyPipeline)!=VK_SUCCESS) return false;

    // Clouds pipeline with alpha blend
    stages[1].module = fsCloud;
    att.blendEnable = VK_TRUE;
    att.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    att.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    att.colorBlendOp = VK_BLEND_OP_ADD;
    att.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    att.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    att.alphaBlendOp = VK_BLEND_OP_ADD;
    if(vkCreateGraphicsPipelines(device,nullptr,1,&pci,nullptr,&cloudPipeline)!=VK_SUCCESS) return false;

    vkDestroyShaderModule(device, fsCloud, nullptr);
    vkDestroyShaderModule(device, fsSky, nullptr);
    vkDestroyShaderModule(device, vs, nullptr);
    return true;
}

void SkyPass::destroy(){
    if(cloudPipeline) vkDestroyPipeline(device, cloudPipeline, nullptr);
    if(skyPipeline) vkDestroyPipeline(device, skyPipeline, nullptr);
    if(pipelineLayout) vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    if(noiseSetLayout) vkDestroyDescriptorSetLayout(device, noiseSetLayout, nullptr);
    device = VK_NULL_HANDLE;
}

void SkyPass::record(VkCommandBuffer cmd, VkDescriptorSet noiseSet, const SkyPushConstants& pc){
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, skyPipeline);
    vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SkyPushConstants), &pc);
    vkCmdDraw(cmd, 3, 1, 0, 0);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, cloudPipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &noiseSet, 0, nullptr);
    vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SkyPushConstants), &pc);
    vkCmdDraw(cmd, 3, 1, 0, 0);
}

} // namespace voxelvk

