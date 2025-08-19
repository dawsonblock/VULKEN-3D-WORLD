#include "lighting_pass.hpp"
#include <vector>
#include <array>

namespace voxelvk {

static VkShaderModule loadShader(VkDevice dev, const char* path){
    FILE* f = fopen(path, "rb");
    if(!f) return VK_NULL_HANDLE;
    fseek(f,0,SEEK_END); long n = ftell(f); fseek(f,0,SEEK_SET);
    std::vector<uint32_t> buf((size_t)n/4u); fread(buf.data(),1,buf.size()*4,f); fclose(f);
    VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    ci.codeSize = buf.size()*4; ci.pCode = buf.data();
    VkShaderModule m; if(vkCreateShaderModule(dev,&ci,nullptr,&m)!=VK_SUCCESS) return VK_NULL_HANDLE; return m;
}

bool LightingPass::init(VkPhysicalDevice phys, VkDevice dev, VkFormat colorFormat, VkFormat depthFormat,
                        VkDescriptorSetLayout materialSet, VkDescriptorSetLayout globalSet){
    device = dev;

    VkShaderModule vs = loadShader(dev, "spv/lighting/pbr_lit.vert.spv");
    if(vs == VK_NULL_HANDLE) vs = loadShader(dev, "shaders/lighting/pbr_lit.vert.glsl.spv");
    VkShaderModule fs = loadShader(dev, "spv/lighting/pbr_lit.frag.spv");
    if(fs == VK_NULL_HANDLE) fs = loadShader(dev, "shaders/lighting/pbr_lit.frag.glsl.spv");
    if(vs == VK_NULL_HANDLE || fs == VK_NULL_HANDLE) return false;

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT; stages[0].module = vs; stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; stages[1].module = fs; stages[1].pName = "main";

    VkVertexInputBindingDescription bind{}; bind.binding=0; bind.stride=sizeof(float)*8; bind.inputRate=VK_VERTEX_INPUT_RATE_VERTEX;
    std::array<VkVertexInputAttributeDescription,3> attrs{};
    attrs[0] = {0,0,VK_FORMAT_R32G32B32_SFLOAT,0};
    attrs[1] = {1,0,VK_FORMAT_R32G32B32_SFLOAT,12};
    attrs[2] = {2,0,VK_FORMAT_R32G32_SFLOAT,24};
    VkPipelineVertexInputStateCreateInfo vi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vi.vertexBindingDescriptionCount=1; vi.pVertexBindingDescriptions=&bind;
    vi.vertexAttributeDescriptionCount=(uint32_t)attrs.size(); vi.pVertexAttributeDescriptions=attrs.data();

    VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rs.polygonMode = VK_POLYGON_MODE_FILL; rs.cullMode = VK_CULL_MODE_BACK_BIT; rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo ds{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    ds.depthTestEnable = VK_TRUE; ds.depthWriteEnable = VK_TRUE; ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkPipelineColorBlendAttachmentState cbatt{}; cbatt.colorWriteMask = 0xF; cbatt.blendEnable = VK_FALSE;
    VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    cb.attachmentCount = 1; cb.pAttachments = &cbatt;

    VkPipelineRenderingCreateInfo rinfo{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    rinfo.colorAttachmentCount = 1; rinfo.pColorAttachmentFormats = &colorFormat;
    rinfo.depthAttachmentFormat = depthFormat;

    VkDescriptorSetLayout setLayouts[2] = {materialSet, globalSet};
    VkPipelineLayoutCreateInfo lci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    lci.setLayoutCount = 2; lci.pSetLayouts = setLayouts;
    if(vkCreatePipelineLayout(dev,&lci,nullptr,&pipelineLayout)!=VK_SUCCESS) return false;

    VkPipelineDynamicStateCreateInfo dyn{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    VkDynamicState dStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    dyn.dynamicStateCount = 2; dyn.pDynamicStates = dStates;

    VkGraphicsPipelineCreateInfo pci{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pci.pNext = &rinfo; pci.stageCount = 2; pci.pStages = stages;
    pci.pVertexInputState = &vi; pci.pInputAssemblyState = &ia;
    pci.pRasterizationState = &rs; pci.pMultisampleState = &ms;
    pci.pDepthStencilState = &ds; pci.pColorBlendState = &cb;
    pci.pDynamicState = &dyn; pci.layout = pipelineLayout;
    pci.renderPass = VK_NULL_HANDLE;
    if(vkCreateGraphicsPipelines(dev,VK_NULL_HANDLE,1,&pci,nullptr,&pipeline)!=VK_SUCCESS) return false;

    vkDestroyShaderModule(dev,vs,nullptr);
    vkDestroyShaderModule(dev,fs,nullptr);
    return true;
}

void LightingPass::destroy(){
    if(pipeline) vkDestroyPipeline(device,pipeline,nullptr);
    if(pipelineLayout) vkDestroyPipelineLayout(device,pipelineLayout,nullptr);
    pipeline = VK_NULL_HANDLE; pipelineLayout = VK_NULL_HANDLE; device = VK_NULL_HANDLE;
}

void LightingPass::record(VkCommandBuffer cmd, VkImageView colorTarget, VkImageView depthTarget,
                          VkExtent2D extent, VkDescriptorSet materialSet, VkDescriptorSet globalSet,
                          const DrawFn& drawScene){
    VkRenderingAttachmentInfo colorAtt{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    colorAtt.imageView = colorTarget; colorAtt.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkRenderingAttachmentInfo depthAtt{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    depthAtt.imageView = depthTarget; depthAtt.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAtt.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; depthAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingInfo ri{VK_STRUCTURE_TYPE_RENDERING_INFO};
    ri.colorAttachmentCount = 1; ri.pColorAttachments = &colorAtt; ri.pDepthAttachment = &depthAtt;
    ri.renderArea.extent = extent; ri.layerCount = 1;

    vkCmdBeginRendering(cmd,&ri);
    vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline);
    VkViewport vp{0,0,(float)extent.width,(float)extent.height,0.0f,1.0f};
    VkRect2D sc{{0,0},extent};
    vkCmdSetViewport(cmd,0,1,&vp); vkCmdSetScissor(cmd,0,1,&sc);
    VkDescriptorSet sets[] = {materialSet, globalSet};
    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelineLayout,0,2,sets,0,nullptr);
    drawScene(cmd);
    vkCmdEndRendering(cmd);
}

} // namespace voxelvk
