#include "voxelize_pass.hpp"
#include "resource_manager.hpp"
#include <stdexcept>
#include <vector>
#include <cstdio>

#include "vk_mem_alloc.h"

namespace voxelvk {

static VkShaderModule loadShader(VkDevice /*dev*/, const char* path);

uint32_t VoxelizePass::findMemoryType(VkPhysicalDevice phys, uint32_t typeBits, VkMemoryPropertyFlags flags){
    VkPhysicalDeviceMemoryProperties memProps{};
    vkGetPhysicalDeviceMemoryProperties(phys, &memProps);

    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {

    for(uint32_t i=0;i<memProps.memoryTypeCount;i++){
        main
        if((typeBits & (1u<<i)) && (memProps.memoryTypes[i].propertyFlags & flags) == flags)
            return i;
    }
    throw std::runtime_error("No suitable memory type");
}


bool VoxelizePass::init(VkPhysicalDevice phys, VkDevice dev, VmaAllocator alloc, uint32_t dimension){
    device = dev;
    allocator = alloc;
    dim = dimension;

bool VoxelizePass::init(VkPhysicalDevice phys, VkDevice dev, VmaAllocator alloc, uint32_t dimension){
    device = dev; allocator = alloc; dim = dimension;
        main
    if(!createImage(phys)) return false;
    if(!createDescriptors()) return false;
    if(!createPipeline(phys)) return false;
    return true;
}

void VoxelizePass::destroy(){
    if(pipeline) vkDestroyPipeline(device, pipeline, nullptr);
    if(pipelineLayout) vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    // Descriptor sets are freed when the descriptor pool is destroyed.
    if(descPool) vkDestroyDescriptorPool(device, descPool, nullptr);
    if(setLayout) vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
    if(voxelView) vkDestroyImageView(device, voxelView, nullptr);
    if(voxelImage) vmaDestroyImage(allocator, voxelImage, voxelAlloc);
}

bool VoxelizePass::createImage(VkPhysicalDevice phys){
    VkImageCreateInfo ci{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = voxelFormat;
    ci.extent = {dim, dim, dim};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VmaAllocationCreateInfo aci{};
    aci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    if(vmaCreateImage(allocator, &ci, &aci, &voxelImage, &voxelAlloc, nullptr) != VK_SUCCESS)
        return false;

    VkImageViewCreateInfo vci{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    vci.image = voxelImage;
    vci.viewType = VK_IMAGE_VIEW_TYPE_3D;
    vci.format = voxelFormat;
    vci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vci.subresourceRange.levelCount = 1;
    vci.subresourceRange.layerCount = 1;
    return gResourceManager.createImageView(&vci, &voxelView) == VK_SUCCESS;
}

bool VoxelizePass::createDescriptors(){
    VkDescriptorSetLayoutBinding b{};
    b.binding = 0;
    b.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    b.descriptorCount = 1;
    b.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo lci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};

    lci.bindingCount = 1;
    lci.pBindings = &b;

    lci.bindingCount = 1; lci.pBindings = &b;
        main
    if(gResourceManager.createDescriptorSetLayout(&lci, &setLayout) != VK_SUCCESS) return false;

    VkDescriptorPoolSize ps{}; ps.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; ps.descriptorCount = 1;
    VkDescriptorPoolCreateInfo pci{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};

    pci.maxSets = 1;
    pci.poolSizeCount = 1;
    pci.pPoolSizes = &ps;

    pci.maxSets = 1; pci.poolSizeCount = 1; pci.pPoolSizes = &ps;
        main
    if(gResourceManager.createDescriptorPool(&pci, &descPool) != VK_SUCCESS) return false;

    VkDescriptorSetAllocateInfo ai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    ai.descriptorPool = descPool; ai.descriptorSetCount = 1; ai.pSetLayouts = &setLayout;
    if(vkAllocateDescriptorSets(device, &ai, &descSet) != VK_SUCCESS) return false;

    VkDescriptorImageInfo ii{}; ii.imageView = voxelView; ii.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkWriteDescriptorSet w{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    w.dstSet = descSet; w.dstBinding = 0; w.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    w.descriptorCount = 1; w.pImageInfo = &ii;
    vkUpdateDescriptorSets(device, 1, &w, 0, nullptr);
    return true;
}

bool VoxelizePass::createPipeline(VkPhysicalDevice phys){
    VkShaderModule vs = loadShader(device, "spv/voxelize/voxelize.vert.spv");
    VkShaderModule fs = loadShader(device, "spv/voxelize/voxelize.frag.spv");
    if(!vs || !fs) return false;

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vs;
    stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fs;
    stages[1].pName = "main";

    VkVertexInputBindingDescription bind{};
    bind.binding = 0; bind.stride = sizeof(glm::vec3); bind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    VkVertexInputAttributeDescription attr{};
    attr.location = 0; attr.binding = 0; attr.format = VK_FORMAT_R32G32B32_SFLOAT; attr.offset = 0;
    VkPipelineVertexInputStateCreateInfo vi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vi.vertexBindingDescriptionCount = 1; vi.pVertexBindingDescriptions = &bind;
    vi.vertexAttributeDescriptionCount = 1; vi.pVertexAttributeDescriptions = &attr;

    VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationConservativeStateCreateInfoEXT cr{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT};
    cr.conservativeRasterizationMode = VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
    cr.extraPrimitiveOverestimationSize = 0.0f;

    VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rs.pNext = &cr; rs.polygonMode = VK_POLYGON_MODE_FILL; rs.cullMode = VK_CULL_MODE_NONE;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; rs.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo ds{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    ds.depthTestEnable = VK_FALSE; ds.depthWriteEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};

    VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};

    dyn.dynamicStateCount = 2;
    dyn.pDynamicStates = dynStates;

    VkPushConstantRange pcr{}; pcr.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pcr.offset = 0;
    pcr.size = sizeof(glm::mat4) + sizeof(int);

    VkPipelineLayoutCreateInfo lci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    lci.setLayoutCount = 1; lci.pSetLayouts = &setLayout;
    lci.pushConstantRangeCount = 1;
    lci.pPushConstantRanges = &pcr;

    dyn.dynamicStateCount = 2; dyn.pDynamicStates = dynStates;

    VkPushConstantRange pcr{}; pcr.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pcr.offset = 0; pcr.size = sizeof(glm::mat4) + sizeof(int);

    VkPipelineLayoutCreateInfo lci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    lci.setLayoutCount = 1; lci.pSetLayouts = &setLayout;
    lci.pushConstantRangeCount = 1; lci.pPushConstantRanges = &pcr;
        main
    if(gResourceManager.createPipelineLayout(&lci, &pipelineLayout) != VK_SUCCESS){
        vkDestroyShaderModule(device, vs, nullptr);
        vkDestroyShaderModule(device, fs, nullptr);
        return false;
    }

    VkPipelineRenderingCreateInfo rinfo{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    rinfo.colorAttachmentCount = 0;
    rinfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    rinfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    VkGraphicsPipelineCreateInfo pci{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pci.pNext = &rinfo;

    pci.stageCount = 2;
    pci.pStages = stages;

    pci.stageCount = 2; pci.pStages = stages;
        main
    pci.pVertexInputState = &vi;
    pci.pInputAssemblyState = &ia;
    pci.pRasterizationState = &rs;
    pci.pMultisampleState = &ms;
    pci.pDepthStencilState = &ds;
    pci.pColorBlendState = &cb;
    pci.pDynamicState = &dyn;
    pci.layout = pipelineLayout;
    pci.renderPass = VK_NULL_HANDLE;
    pci.subpass = 0;

    bool ok = gResourceManager.createGraphicsPipeline(&pci, &pipeline) == VK_SUCCESS;
    vkDestroyShaderModule(device, vs, nullptr);
    vkDestroyShaderModule(device, fs, nullptr);
    return ok;
}

void VoxelizePass::record(VkCommandBuffer cmd, const RecordVoxelDrawFn& drawScene){
    VkViewport vp{0,0,(float)dim,(float)dim,0.0f,1.0f};
    VkRect2D sc{{0,0},{dim,dim}};

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descSet, 0, nullptr);
    vkCmdSetViewport(cmd, 0, 1, &vp);
    vkCmdSetScissor(cmd, 0, 1, &sc);

    for(uint32_t z=0; z<dim; ++z){

        struct Push { glm::mat4 mvp; int slice; } pc{};
        pc.mvp = glm::mat4(1.0f);

        Push pc{};
        pc.mvp = mvp;
        main
        pc.slice = (int)z;
        vkCmdPushConstants(cmd, pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                           0, sizeof(Push), &pc);

        VkRenderingInfo ri{VK_STRUCTURE_TYPE_RENDERING_INFO};
        ri.renderArea = sc;
        ri.layerCount = 1;
        ri.colorAttachmentCount = 0;
        vkCmdBeginRendering(cmd, &ri);
        drawScene(cmd, (int)z);
        vkCmdEndRendering(cmd);
    }
}

static std::vector<char> readFile(const char* path){
    std::vector<char> data;
    FILE* f = std::fopen(path, "rb");
    if(!f) return data;
    std::fseek(f, 0, SEEK_END);
    long n = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    data.resize(n);
    std::fread(data.data(), 1, n, f);
    std::fclose(f);
    return data;
}

static VkShaderModule loadShader(VkDevice /*dev*/, const char* path){
    auto bytes = readFile(path);

    if(bytes.empty()) return VK_NULL_HANDLE;

    if(bytes.empty()) {
        std::fprintf(stderr, "Error: Failed to load shader file '%s'\n", path);
        return VK_NULL_HANDLE;
    }
        main
    VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    ci.codeSize = bytes.size();
    ci.pCode = reinterpret_cast<const uint32_t*>(bytes.data());
    VkShaderModule m;
    if(gResourceManager.createShaderModule(&ci, &m) != VK_SUCCESS) return VK_NULL_HANDLE;
    return m;
}

} // namespace voxelvk
