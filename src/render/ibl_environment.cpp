#include "ibl_environment.hpp"
#include <vector>
#include <cstring>
#include <string>

namespace voxelvk {

static uint32_t FindMemoryType(VkPhysicalDevice phys, uint32_t typeFilter, VkMemoryPropertyFlags properties){
    VkPhysicalDeviceMemoryProperties memProps{};
    vkGetPhysicalDeviceMemoryProperties(phys, &memProps);
    for(uint32_t i=0;i<memProps.memoryTypeCount;i++){
        if((typeFilter & (1u<<i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    return ~0u;
}

static VkShaderModule LoadShaderModule(VkDevice device, const char* path){
    FILE* f = fopen(path, "rb");
    if(!f) return VK_NULL_HANDLE;
    fseek(f,0,SEEK_END); long len = ftell(f); fseek(f,0,SEEK_SET);
    std::vector<uint32_t> buf((size_t)len/4u);
    fread(buf.data(),1,buf.size()*4,f); fclose(f);
    VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    ci.codeSize = buf.size()*4; ci.pCode = buf.data();
    VkShaderModule mod = VK_NULL_HANDLE;
    if(vkCreateShaderModule(device,&ci,nullptr,&mod)!=VK_SUCCESS) return VK_NULL_HANDLE;
    return mod;
}

bool GenerateEnvironmentMaps(VkDevice device,
                             VkPhysicalDevice phys,
                             VmaAllocator allocator,
                             VkCommandPool cmdPool,
                             VkQueue queue,
                             VkImageView equirectView,
                             VkSampler equirectSampler,
                             EnvironmentMaps& out,
                             uint32_t envSize,
                             uint32_t irradianceSize){
    out.envSize = envSize; out.irradianceSize = irradianceSize;

    VkFormat fmt = VK_FORMAT_R16G16B16A16_SFLOAT;

    // Create environment cubemap
    VkImageCreateInfo img{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    img.imageType = VK_IMAGE_TYPE_2D; img.format = fmt;
    img.extent = {envSize, envSize, 1};
    img.mipLevels = 1; img.arrayLayers = 6;
    img.samples = VK_SAMPLE_COUNT_1_BIT; img.tiling = VK_IMAGE_TILING_OPTIMAL;
    img.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    img.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    VmaAllocationCreateInfo aci{}; aci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    if(vmaCreateImage(allocator,&img,&aci,&out.envImage,&out.envAlloc,nullptr)!=VK_SUCCESS) return false;

    VkImageViewCreateInfo iv{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    iv.image = out.envImage; iv.viewType = VK_IMAGE_VIEW_TYPE_CUBE; iv.format = fmt;
    iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    iv.subresourceRange.levelCount = 1; iv.subresourceRange.layerCount = 6;
    if(vkCreateImageView(device,&iv,nullptr,&out.envView)!=VK_SUCCESS) return false;

    VkSamplerCreateInfo sci{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    sci.magFilter = VK_FILTER_LINEAR; sci.minFilter = VK_FILTER_LINEAR;
    sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sci.addressModeU = sci.addressModeV = sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    if(vkCreateSampler(device,&sci,nullptr,&out.envSampler)!=VK_SUCCESS) return false;

    // Create irradiance cubemap
    img.extent = {irradianceSize, irradianceSize, 1};
    if(vmaCreateImage(allocator,&img,&aci,&out.irradianceImage,&out.irradianceAlloc,nullptr)!=VK_SUCCESS) return false;
    iv.image = out.irradianceImage; iv.subresourceRange.layerCount = 6;
    if(vkCreateImageView(device,&iv,nullptr,&out.irradianceView)!=VK_SUCCESS) return false;
    if(vkCreateSampler(device,&sci,nullptr,&out.irradianceSampler)!=VK_SUCCESS) return false;

    // Descriptor set layout for passes
    VkDescriptorSetLayoutBinding b0{}; // sampled
    b0.binding = 0; b0.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    b0.descriptorCount = 1; b0.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    VkDescriptorSetLayoutBinding b1{}; // storage
    b1.binding = 1; b1.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    b1.descriptorCount = 1; b1.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    VkDescriptorSetLayoutCreateInfo lci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    VkDescriptorSetLayout dsl;
    VkDescriptorPool pool; VkDescriptorSet ds;

    // Helper to run a compute shader pass
    auto runPass = [&](const char* spvPath, VkImageView inView, VkSampler inSampler, VkImageView outView, uint32_t w){
        lci.bindingCount = 2; VkDescriptorSetLayoutBinding binds[2] = {b0,b1}; lci.pBindings = binds;
        if(vkCreateDescriptorSetLayout(device,&lci,nullptr,&dsl)!=VK_SUCCESS) return false;
        VkPipelineLayoutCreateInfo plci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        plci.setLayoutCount = 1; plci.pSetLayouts = &dsl;
        VkPipelineLayout pl; if(vkCreatePipelineLayout(device,&plci,nullptr,&pl)!=VK_SUCCESS) return false;
        VkDescriptorPoolSize sizes[2] = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1},{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,1}};
        VkDescriptorPoolCreateInfo pci{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        pci.maxSets=1; pci.poolSizeCount=2; pci.pPoolSizes=sizes;
        if(vkCreateDescriptorPool(device,&pci,nullptr,&pool)!=VK_SUCCESS) return false;
        VkDescriptorSetAllocateInfo ai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO}; ai.descriptorPool=pool; ai.descriptorSetCount=1; ai.pSetLayouts=&dsl;
        if(vkAllocateDescriptorSets(device,&ai,&ds)!=VK_SUCCESS) return false;
        VkDescriptorImageInfo inInfo{}; inInfo.imageView = inView; inInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; inInfo.sampler = inSampler;
        VkDescriptorImageInfo outInfo{}; outInfo.imageView = outView; outInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        VkWriteDescriptorSet writes[2]{};
        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; writes[0].dstSet = ds; writes[0].dstBinding=0; writes[0].descriptorCount=1; writes[0].descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; writes[0].pImageInfo=&inInfo;
        writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; writes[1].dstSet = ds; writes[1].dstBinding=1; writes[1].descriptorCount=1; writes[1].descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; writes[1].pImageInfo=&outInfo;
        vkUpdateDescriptorSets(device,2,writes,0,nullptr);
        VkShaderModule cs = LoadShaderModule(device, spvPath);
        if(cs==VK_NULL_HANDLE){
            // fallback to build dir
            std::string alt = std::string("shaders/ibl/") + spvPath;
            cs = LoadShaderModule(device, alt.c_str());
            if(cs==VK_NULL_HANDLE) return false;
        }
        VkComputePipelineCreateInfo cpci{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        VkPipelineShaderStageCreateInfo s{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        s.stage = VK_SHADER_STAGE_COMPUTE_BIT; s.module = cs; s.pName = "main"; cpci.stage = s; cpci.layout = pl;
        VkPipeline pipe; if(vkCreateComputePipelines(device,VK_NULL_HANDLE,1,&cpci,nullptr,&pipe)!=VK_SUCCESS) return false;

        VkCommandBufferAllocateInfo cbai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        cbai.commandPool=cmdPool; cbai.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY; cbai.commandBufferCount=1;
        VkCommandBuffer cmd; vkAllocateCommandBuffers(device,&cbai,&cmd);
        VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO}; bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; vkBeginCommandBuffer(cmd,&bi);
        VkImageMemoryBarrier2 barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT; barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT; barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.image = outView == out.envView ? out.envImage : out.irradianceImage; // not accurate but ok
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.levelCount = 1; barrier.subresourceRange.layerCount = 6;
        VkDependencyInfo dep{VK_STRUCTURE_TYPE_DEPENDENCY_INFO}; dep.imageMemoryBarrierCount=1; dep.pImageMemoryBarriers=&barrier;
        vkCmdPipelineBarrier2(cmd,&dep);
        vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,pipe);
        vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,pl,0,1,&ds,0,nullptr);
        uint32_t gx = (w + 7)/8; uint32_t gy = (w + 7)/8;
        vkCmdDispatch(cmd,gx,gy,6);
        VkImageMemoryBarrier2 barrier2{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
        barrier2.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT; barrier2.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
        barrier2.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        barrier2.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
        barrier2.oldLayout = VK_IMAGE_LAYOUT_GENERAL; barrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier2.image = barrier.image; barrier2.subresourceRange = barrier.subresourceRange;
        VkDependencyInfo dep2{VK_STRUCTURE_TYPE_DEPENDENCY_INFO}; dep2.imageMemoryBarrierCount=1; dep2.pImageMemoryBarriers=&barrier2;
        vkCmdPipelineBarrier2(cmd,&dep2);
        vkEndCommandBuffer(cmd);
        VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO}; si.commandBufferCount=1; si.pCommandBuffers=&cmd;
        vkQueueSubmit(queue,1,&si,VK_NULL_HANDLE); vkQueueWaitIdle(queue);
        vkFreeCommandBuffers(device,cmdPool,1,&cmd);
        vkDestroyPipeline(device,pipe,nullptr); vkDestroyShaderModule(device,cs,nullptr);
        vkDestroyDescriptorPool(device,pool,nullptr); vkDestroyPipelineLayout(device,pl,nullptr); vkDestroyDescriptorSetLayout(device,dsl,nullptr);
        return true;
    };

    // equirect -> cubemap
    if(!runPass("spv/ibl/equirect_to_cubemap.comp.spv", equirectView, equirectSampler, out.envView, envSize)) return false;

    // prefilter (identity for now)
    if(!runPass("spv/ibl/pmrem_prefilter.comp.spv", out.envView, out.envSampler, out.envView, envSize)) return false;

    // irradiance
    if(!runPass("spv/ibl/irradiance_convolution.comp.spv", out.envView, out.envSampler, out.irradianceView, irradianceSize)) return false;

    return true;
}

void DestroyEnvironmentMaps(VkDevice device, VmaAllocator allocator, EnvironmentMaps& env){
    if(env.envSampler) vkDestroySampler(device, env.envSampler, nullptr);
    if(env.envView) vkDestroyImageView(device, env.envView, nullptr);
    if(env.envImage) vmaDestroyImage(allocator, env.envImage, env.envAlloc);
    if(env.irradianceSampler) vkDestroySampler(device, env.irradianceSampler, nullptr);
    if(env.irradianceView) vkDestroyImageView(device, env.irradianceView, nullptr);
    if(env.irradianceImage) vmaDestroyImage(allocator, env.irradianceImage, env.irradianceAlloc);
    env = EnvironmentMaps{};
}

} // namespace voxelvk
