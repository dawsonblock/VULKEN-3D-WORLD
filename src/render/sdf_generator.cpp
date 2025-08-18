#include "sdf_generator.hpp"
#include <vector>
#include <algorithm>
#include <cstring>
namespace voxelvk {
static VkShaderModule LoadShaderModuleFromFile(VkDevice device, const char* path){
    FILE* f = fopen(path, "rb");
    if(!f) return VK_NULL_HANDLE;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<uint32_t> buf((size_t)len/4u);
    fread(buf.data(),1,buf.size()*4,f); fclose(f);
    VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    ci.codeSize = buf.size()*4; ci.pCode = buf.data();
    VkShaderModule mod = VK_NULL_HANDLE;
    if(!f) {
        fprintf(stderr, "Error: Failed to open shader file '%s'\n", path);
        return VK_NULL_HANDLE;
    }
    fseek(f,0,SEEK_END); long len = ftell(f); fseek(f,0,SEEK_SET);
    std::vector<uint32_t> buf((size_t)len/4u);
    fread(buf.data(),1,buf.size()*4,f); fclose(f);
    VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    ci.codeSize = buf.size()*4; ci.pCode = buf.data();
    VkShaderModule mod = VK_NULL_HANDLE;
    if(vkCreateShaderModule(device,&ci,nullptr,&mod)!=VK_SUCCESS) {
        fprintf(stderr, "Error: Failed to create shader module from file '%s'\n", path);
        return VK_NULL_HANDLE;
    }
    return mod;
}
bool GenerateSDF(VkDevice device,
                 VkPhysicalDevice /*phys*/,
                 VmaAllocator allocator,
                 VkCommandPool cmdPool,
                 VkQueue queue,
                 VkImage seedImage,
                 VkImageView seedView,
                 VkExtent3D extent,
                 VkFormat outFormat,
                 SDFVolume& out){
    (void)seedImage;
    out.width = extent.width;
    out.height = extent.height;
    out.depth = extent.depth;
    out.format = outFormat;
    // Create output image
    VkImageCreateInfo img{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    img.imageType = VK_IMAGE_TYPE_3D; img.format = outFormat; img.extent = extent;
    img.mipLevels=1; img.arrayLayers=1; img.samples=VK_SAMPLE_COUNT_1_BIT; img.tiling=VK_IMAGE_TILING_OPTIMAL;
    img.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VmaAllocationCreateInfo aci{}; aci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; aci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    if(vmaCreateImage(allocator,&img,&aci,&out.image,&out.allocation,nullptr)!=VK_SUCCESS) return false;
    VkImageViewCreateInfo ivci{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    ivci.image=out.image; ivci.viewType=VK_IMAGE_VIEW_TYPE_3D; ivci.format=out.format;
    ivci.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT; ivci.subresourceRange.levelCount=1; ivci.subresourceRange.layerCount=1;
    if(vkCreateImageView(device,&ivci,nullptr,&out.view)!=VK_SUCCESS) return false;
    // Temp ping/pong images (RGBA16F)
    VkImageCreateInfo tmpImg = img; tmpImg.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    VkImage tmpA, tmpB; VmaAllocation tmpAAlloc, tmpBAlloc; VkImageView tmpAView, tmpBView;
    if(vmaCreateImage(allocator,&tmpImg,&aci,&tmpA,&tmpAAlloc,nullptr)!=VK_SUCCESS) return false;
    if(vmaCreateImage(allocator,&tmpImg,&aci,&tmpB,&tmpBAlloc,nullptr)!=VK_SUCCESS) return false;
    ivci.image=tmpA; ivci.viewType=VK_IMAGE_VIEW_TYPE_3D; ivci.format=VK_FORMAT_R16G16B16A16_SFLOAT;
    if(vkCreateImageView(device,&ivci,nullptr,&tmpAView)!=VK_SUCCESS) return false;
    ivci.image=tmpB; if(vkCreateImageView(device,&ivci,nullptr,&tmpBView)!=VK_SUCCESS) return false;
    // Descriptor set layout
    VkDescriptorSetLayoutBinding binds[2]{};
    binds[0].binding=0; binds[0].descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; binds[0].descriptorCount=1; binds[0].stageFlags=VK_SHADER_STAGE_COMPUTE_BIT;
    binds[1].binding=1; binds[1].descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; binds[1].descriptorCount=1; binds[1].stageFlags=VK_SHADER_STAGE_COMPUTE_BIT;
    VkDescriptorSetLayoutCreateInfo dslci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    dslci.bindingCount = 2;
    dslci.pBindings = binds;
    VkDescriptorSetLayout dsl;
    if (vkCreateDescriptorSetLayout(device, &dslci, nullptr, &dsl) != VK_SUCCESS)
        return false;
    VkPushConstantRange pcr{VK_SHADER_STAGE_COMPUTE_BIT,0,sizeof(int)*2};
    VkPipelineLayoutCreateInfo plci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    plci.setLayoutCount=1; plci.pSetLayouts=&dsl; plci.pushConstantRangeCount=1; plci.pPushConstantRanges=&pcr;
    VkPipelineLayout pl; if(vkCreatePipelineLayout(device,&plci,nullptr,&pl)!=VK_SUCCESS) return false;
    VkDescriptorPoolSize dps{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,2};
    VkDescriptorPoolCreateInfo dpci{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO}; dpci.maxSets=1; dpci.poolSizeCount=1; dpci.pPoolSizes=&dps;
    VkDescriptorPool pool; if(vkCreateDescriptorPool(device,&dpci,nullptr,&pool)!=VK_SUCCESS) return false;
    VkDescriptorSetAllocateInfo dsai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    dsai.descriptorPool = pool;
    dsai.descriptorSetCount = 1;
    dsai.pSetLayouts = &dsl;
    VkDescriptorSet ds;
    if (vkAllocateDescriptorSets(device, &dsai, &ds) != VK_SUCCESS) return false;
    auto UpdateDS=[&](VkImageView src, VkImageView dst){
        VkDescriptorImageInfo infos[2]{};
        infos[0].imageView=src; infos[0].imageLayout=VK_IMAGE_LAYOUT_GENERAL;
        infos[1].imageView=dst; infos[1].imageLayout=VK_IMAGE_LAYOUT_GENERAL;
        VkWriteDescriptorSet writes[2]{ {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET}, {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET} };
        writes[0].dstSet=ds; writes[0].dstBinding=0; writes[0].descriptorCount=1; writes[0].descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; writes[0].pImageInfo=&infos[0];
        writes[1].dstSet=ds; writes[1].dstBinding=1; writes[1].descriptorCount=1; writes[1].descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; writes[1].pImageInfo=&infos[1];
        vkUpdateDescriptorSets(device,2,writes,0,nullptr);
    };
    // Shader module
    VkShaderModule cs = LoadShaderModuleFromFile(device,"spv/sdf_jump_flood.comp.spv");
    if(cs==VK_NULL_HANDLE) cs = LoadShaderModuleFromFile(device,"shaders/sdf_jump_flood.comp.glsl.spv");
    if(cs==VK_NULL_HANDLE) return false;
    VkComputePipelineCreateInfo cpci{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    VkPipelineShaderStageCreateInfo ss{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    ss.stage=VK_SHADER_STAGE_COMPUTE_BIT; ss.module=cs; ss.pName="main";
    cpci.stage = ss;
    cpci.layout = pl;
    VkPipeline pipe;
    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &cpci, nullptr, &pipe) != VK_SUCCESS) {
        vkDestroyShaderModule(device, cs, nullptr);
        return false;
    }
    // Command buffer
    VkCommandBufferAllocateInfo cbai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO}; cbai.commandPool=cmdPool; cbai.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY; cbai.commandBufferCount=1; VkCommandBuffer cmd; vkAllocateCommandBuffers(device,&cbai,&cmd);
    VkCommandBufferBeginInfo cbbi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO}; vkBeginCommandBuffer(cmd,&cbbi);
    auto barrier=[&](VkImage img){
        VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        b.srcAccessMask=0; b.dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT;
        b.oldLayout=VK_IMAGE_LAYOUT_UNDEFINED; b.newLayout=VK_IMAGE_LAYOUT_GENERAL;
        b.image=img; b.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT; b.subresourceRange.levelCount=1; b.subresourceRange.layerCount=1;
        vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,0,nullptr,0,nullptr,1,&b);
    };
    barrier(out.image); barrier(tmpA); barrier(tmpB);
    // Ping from seed
    int maxDim = std::max({ (int)extent.width, (int)extent.height, (int)extent.depth });
    int step = maxDim / 2; bool useTmpA=true; VkImageView srcView=seedView;
    while(step>0){
        VkImageView dstView=useTmpA?tmpAView:tmpBView;
        UpdateDS(srcView,dstView);
        vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,pipe);
        int pc[2]={step,0};
        vkCmdPushConstants(cmd,pl,VK_SHADER_STAGE_COMPUTE_BIT,0,sizeof(pc),pc);
        vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,pl,0,1,&ds,0,nullptr);
        vkCmdDispatch(cmd,(extent.width+3)/4,(extent.height+3)/4,(extent.depth+3)/4);
        VkImageMemoryBarrier bb{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        bb.srcAccessMask=VK_ACCESS_SHADER_WRITE_BIT; bb.dstAccessMask=VK_ACCESS_SHADER_READ_BIT;
        bb.oldLayout=VK_IMAGE_LAYOUT_GENERAL; bb.newLayout=VK_IMAGE_LAYOUT_GENERAL; bb.image=dstView==tmpAView?tmpA:tmpB; bb.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT; bb.subresourceRange.levelCount=1; bb.subresourceRange.layerCount=1;
        vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,0,nullptr,0,nullptr,1,&bb);
        srcView=dstView; useTmpA=!useTmpA; step/=2;
    }
    // Final pass to out
    UpdateDS(srcView,out.view);
    vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,pipe);
    int pcFin[2]={1,1};
    vkCmdPushConstants(cmd,pl,VK_SHADER_STAGE_COMPUTE_BIT,0,sizeof(pcFin),pcFin);
    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,pl,0,1,&ds,0,nullptr);
    vkCmdDispatch(cmd,(extent.width+3)/4,(extent.height+3)/4,(extent.depth+3)/4);
    vkEndCommandBuffer(cmd);
    VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    si.commandBufferCount = 1;
    si.pCommandBuffers = &cmd;
    vkQueueSubmit(queue, 1, &si, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    vkDestroyShaderModule(device,cs,nullptr);
    vkDestroyPipeline(device,pipe,nullptr);
    vkDestroyPipelineLayout(device,pl,nullptr);
    vkDestroyDescriptorSetLayout(device,dsl,nullptr);
    vkDestroyDescriptorPool(device,pool,nullptr);
    vkDestroyImageView(device, tmpAView, nullptr);
    vkDestroyImageView(device, tmpBView, nullptr);
    vmaDestroyImage(allocator, tmpA, tmpAAlloc);
    vmaDestroyImage(allocator, tmpB, tmpBAlloc);
    return true;
}
void DestroySDFVolume(VkDevice device, VmaAllocator allocator, SDFVolume& vol){
    if(vol.view) vkDestroyImageView(device,vol.view,nullptr);
    if(vol.image) vmaDestroyImage(allocator,vol.image,vol.allocation);
    vol = {};
}
} // namespace voxelvk
