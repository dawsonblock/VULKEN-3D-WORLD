#include "ibl_pipeline.hpp"
#include <vector>
#include <cmath>
#include <cassert>
#include <cstring>

namespace voxelvk {

namespace {

VkShaderModule loadShader(VkDevice device, const char* path){
    FILE* f = std::fopen(path, "rb");
    if(!f) return VK_NULL_HANDLE;
    std::fseek(f,0,SEEK_END);
    long n = std::ftell(f);
    std::fseek(f,0,SEEK_SET);
    std::vector<uint32_t> buf((size_t)n/4u);
    std::fread(buf.data(),1,buf.size()*4,f);
    std::fclose(f);
    VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    ci.codeSize = buf.size()*4; ci.pCode = buf.data();
    VkShaderModule mod = VK_NULL_HANDLE;
    if(vkCreateShaderModule(device,&ci,nullptr,&mod)!=VK_SUCCESS) return VK_NULL_HANDLE;
    return mod;
}

struct PushPrefilter {
    float roughness;
    uint32_t face;
};

}

bool GenerateIBLMaps(VkDevice device,
                     VkPhysicalDevice /*phys*/,
                     VmaAllocator allocator,
                     VkCommandPool cmdPool,
                     VkQueue queue,
                     VkImageView envView,
                     VkSampler envSampler,
                     IBLMaps& outMaps,
                     IBLDescriptor& outDesc,
                     VkFormat format,
                     uint32_t irradianceSize,
                     uint32_t prefilterSize){
    outMaps = IBLMaps{};
    outDesc = IBLDescriptor{};

    outMaps.irradianceSize = irradianceSize;
    outMaps.prefilterSize = prefilterSize;
    outMaps.prefilterMips = (uint32_t)std::floor(std::log2((double)prefilterSize)) + 1u;

    // Create irradiance image
    VkImageCreateInfo img{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    img.imageType = VK_IMAGE_TYPE_2D;
    img.format = format;
    img.extent = {irradianceSize,irradianceSize,1};
    img.mipLevels = 1;
    img.arrayLayers = 6;
    img.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    img.samples = VK_SAMPLE_COUNT_1_BIT;
    img.tiling = VK_IMAGE_TILING_OPTIMAL;
    img.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    img.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VmaAllocationCreateInfo aci{}; aci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; aci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    if(vmaCreateImage(allocator,&img,&aci,&outMaps.irradianceImage,nullptr,nullptr)!=VK_SUCCESS) return false;

    VkImageViewCreateInfo iv{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    iv.image = outMaps.irradianceImage;
    iv.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    iv.format = format;
    iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    iv.subresourceRange.levelCount = 1;
    iv.subresourceRange.layerCount = 6;
    if(vkCreateImageView(device,&iv,nullptr,&outMaps.irradianceView)!=VK_SUCCESS) return false;

    VkSamplerCreateInfo sci{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    sci.magFilter = VK_FILTER_LINEAR; sci.minFilter = VK_FILTER_LINEAR;
    sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sci.addressModeU = sci.addressModeV = sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    if(vkCreateSampler(device,&sci,nullptr,&outMaps.irradianceSampler)!=VK_SUCCESS) return false;

    // Create prefilter image
    img.extent = {prefilterSize,prefilterSize,1};
    img.mipLevels = outMaps.prefilterMips;
    if(vmaCreateImage(allocator,&img,&aci,&outMaps.prefilterImage,nullptr,nullptr)!=VK_SUCCESS) return false;

    iv.image = outMaps.prefilterImage;
    iv.subresourceRange.levelCount = outMaps.prefilterMips;
    if(vkCreateImageView(device,&iv,nullptr,&outMaps.prefilterView)!=VK_SUCCESS) return false;

    sci.minLod = 0.0f; sci.maxLod = (float)(outMaps.prefilterMips-1);
    if(vkCreateSampler(device,&sci,nullptr,&outMaps.prefilterSampler)!=VK_SUCCESS) return false;

    // Descriptor set layout for compute passes (env sampler + storage image)
    VkDescriptorSetLayoutBinding bindings[2]{};
    bindings[0].binding = 0; bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].descriptorCount = 1; bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings[1].binding = 1; bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindings[1].descriptorCount = 1; bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    VkDescriptorSetLayoutCreateInfo lci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    lci.bindingCount = 2; lci.pBindings = bindings;
    VkDescriptorSetLayout computeLayout;
    if(vkCreateDescriptorSetLayout(device,&lci,nullptr,&computeLayout)!=VK_SUCCESS) return false;

    VkPushConstantRange pc{}; pc.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT; pc.offset = 0; pc.size = sizeof(PushPrefilter);
    VkPipelineLayoutCreateInfo plci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    plci.setLayoutCount = 1; plci.pSetLayouts = &computeLayout;
    plci.pushConstantRangeCount = 1; plci.pPushConstantRanges = &pc;
    VkPipelineLayout computePL;
    if(vkCreatePipelineLayout(device,&plci,nullptr,&computePL)!=VK_SUCCESS){
        vkDestroyDescriptorSetLayout(device,computeLayout,nullptr);
        return false;
    }

    // Descriptor pool and set for compute
    VkDescriptorPoolSize poolSizes[2]{{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1},{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,1}};
    VkDescriptorPoolCreateInfo dpci{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    dpci.maxSets = 1; dpci.poolSizeCount = 2; dpci.pPoolSizes = poolSizes;
    VkDescriptorPool computePool;
    if(vkCreateDescriptorPool(device,&dpci,nullptr,&computePool)!=VK_SUCCESS){
        vkDestroyPipelineLayout(device,computePL,nullptr);
        vkDestroyDescriptorSetLayout(device,computeLayout,nullptr);
        return false;
    }
    VkDescriptorSetAllocateInfo dsai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    dsai.descriptorPool = computePool; dsai.descriptorSetCount = 1; dsai.pSetLayouts = &computeLayout;
    VkDescriptorSet computeSet;
    if(vkAllocateDescriptorSets(device,&dsai,&computeSet)!=VK_SUCCESS){
        vkDestroyDescriptorPool(device,computePool,nullptr);
        vkDestroyPipelineLayout(device,computePL,nullptr);
        vkDestroyDescriptorSetLayout(device,computeLayout,nullptr);
        return false;
    }

    // Update env sampler binding once
    VkDescriptorImageInfo envInfo{}; envInfo.imageView = envView; envInfo.sampler = envSampler; envInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet w0{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    w0.dstSet = computeSet; w0.dstBinding = 0; w0.descriptorCount = 1; w0.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; w0.pImageInfo = &envInfo;
    vkUpdateDescriptorSets(device,1,&w0,0,nullptr);

    // Load shaders
    VkShaderModule irrCS = loadShader(device,"spv/ibl/irradiance.comp.spv");
    if(irrCS==VK_NULL_HANDLE) irrCS = loadShader(device,"shaders/ibl/irradiance.comp.glsl.spv");
    VkShaderModule preCS = loadShader(device,"spv/ibl/prefilter_env.comp.spv");
    if(preCS==VK_NULL_HANDLE) preCS = loadShader(device,"shaders/ibl/prefilter_env.comp.glsl.spv");
    if(irrCS==VK_NULL_HANDLE || preCS==VK_NULL_HANDLE){
        vkDestroyDescriptorPool(device,computePool,nullptr);
        vkDestroyPipelineLayout(device,computePL,nullptr);
        vkDestroyDescriptorSetLayout(device,computeLayout,nullptr);
        return false;
    }

    VkPipelineShaderStageCreateInfo stage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    stage.stage = VK_SHADER_STAGE_COMPUTE_BIT; stage.pName = "main";
    VkComputePipelineCreateInfo cpci{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    cpci.layout = computePL; cpci.stage = stage;

    stage.module = irrCS;
    VkPipeline irrPipe;
    if(vkCreateComputePipelines(device,VK_NULL_HANDLE,1,&cpci,nullptr,&irrPipe)!=VK_SUCCESS){
        vkDestroyShaderModule(device,irrCS,nullptr); vkDestroyShaderModule(device,preCS,nullptr);
        vkDestroyDescriptorPool(device,computePool,nullptr);
        vkDestroyPipelineLayout(device,computePL,nullptr);
        vkDestroyDescriptorSetLayout(device,computeLayout,nullptr);
        return false;
    }

    stage.module = preCS;
    VkPipeline prePipe;
    if(vkCreateComputePipelines(device,VK_NULL_HANDLE,1,&cpci,nullptr,&prePipe)!=VK_SUCCESS){
        vkDestroyPipeline(device,irrPipe,nullptr);
        vkDestroyShaderModule(device,irrCS,nullptr); vkDestroyShaderModule(device,preCS,nullptr);
        vkDestroyDescriptorPool(device,computePool,nullptr);
        vkDestroyPipelineLayout(device,computePL,nullptr);
        vkDestroyDescriptorSetLayout(device,computeLayout,nullptr);
        return false;
    }

    // Command buffer
    VkCommandBufferAllocateInfo cbai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cbai.commandPool = cmdPool; cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; cbai.commandBufferCount = 1;
    VkCommandBuffer cmd; vkAllocateCommandBuffers(device,&cbai,&cmd);

    VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd,&bi);

    // Irradiance faces
    for(uint32_t face=0; face<6; ++face){
        VkImageViewCreateInfo faceViewCI{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        faceViewCI.image = outMaps.irradianceImage;
        faceViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        faceViewCI.format = format;
        faceViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        faceViewCI.subresourceRange.baseMipLevel = 0;
        faceViewCI.subresourceRange.levelCount = 1;
        faceViewCI.subresourceRange.baseArrayLayer = face;
        faceViewCI.subresourceRange.layerCount = 1;
        VkImageView faceView;
        vkCreateImageView(device,&faceViewCI,nullptr,&faceView);

        VkDescriptorImageInfo outInfo{}; outInfo.imageView = faceView; outInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        VkWriteDescriptorSet w1{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        w1.dstSet = computeSet; w1.dstBinding = 1; w1.descriptorCount = 1; w1.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; w1.pImageInfo = &outInfo;
        vkUpdateDescriptorSets(device,1,&w1,0,nullptr);

        VkImageMemoryBarrier2 b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
        b.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT; b.srcAccessMask = 0;
        b.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT; b.dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
        b.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; b.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        b.image = outMaps.irradianceImage;
        b.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        b.subresourceRange.baseMipLevel = 0; b.subresourceRange.levelCount = 1;
        b.subresourceRange.baseArrayLayer = face; b.subresourceRange.layerCount = 1;
        VkDependencyInfo dep{VK_STRUCTURE_TYPE_DEPENDENCY_INFO}; dep.imageMemoryBarrierCount=1; dep.pImageMemoryBarriers=&b;
        vkCmdPipelineBarrier2(cmd,&dep);

        vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,irrPipe);
        vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,computePL,0,1,&computeSet,0,nullptr);
        PushPrefilter pcData{0.0f,face};
        vkCmdPushConstants(cmd,computePL,VK_SHADER_STAGE_COMPUTE_BIT,0,sizeof(PushPrefilter),&pcData);
        uint32_t groups = (irradianceSize+7)/8;
        vkCmdDispatch(cmd,groups,groups,1);

        vkDestroyImageView(device,faceView,nullptr);
    }

    // Barrier to make irradiance readable
    VkImageMemoryBarrier2 bEnd{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
    bEnd.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT; bEnd.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
    bEnd.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    bEnd.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    bEnd.oldLayout = VK_IMAGE_LAYOUT_GENERAL; bEnd.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    bEnd.image = outMaps.irradianceImage;
    bEnd.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bEnd.subresourceRange.levelCount = 1; bEnd.subresourceRange.layerCount = 6;
    VkDependencyInfo depEnd{VK_STRUCTURE_TYPE_DEPENDENCY_INFO}; depEnd.imageMemoryBarrierCount=1; depEnd.pImageMemoryBarriers=&bEnd;
    vkCmdPipelineBarrier2(cmd,&depEnd);

    // Prefilter map
    for(uint32_t mip=0; mip<outMaps.prefilterMips; ++mip){
        uint32_t size = prefilterSize >> mip;
        if(size==0) size=1;
        uint32_t groups = (size+7)/8;
        float rough = (outMaps.prefilterMips>1)?((float)mip/(float)(outMaps.prefilterMips-1)) : 0.0f;
        for(uint32_t face=0; face<6; ++face){
            VkImageViewCreateInfo faceViewCI{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            faceViewCI.image = outMaps.prefilterImage;
            faceViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
            faceViewCI.format = format;
            faceViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            faceViewCI.subresourceRange.baseMipLevel = mip; faceViewCI.subresourceRange.levelCount = 1;
            faceViewCI.subresourceRange.baseArrayLayer = face; faceViewCI.subresourceRange.layerCount = 1;
            VkImageView faceView;
            vkCreateImageView(device,&faceViewCI,nullptr,&faceView);

            VkDescriptorImageInfo outInfo{}; outInfo.imageView = faceView; outInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            VkWriteDescriptorSet w1{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            w1.dstSet = computeSet; w1.dstBinding = 1; w1.descriptorCount = 1; w1.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; w1.pImageInfo = &outInfo;
            vkUpdateDescriptorSets(device,1,&w1,0,nullptr);

            VkImageMemoryBarrier2 b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
            b.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT; b.srcAccessMask = 0;
            b.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT; b.dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
            b.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; b.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            b.image = outMaps.prefilterImage;
            b.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            b.subresourceRange.baseMipLevel = mip; b.subresourceRange.levelCount = 1;
            b.subresourceRange.baseArrayLayer = face; b.subresourceRange.layerCount = 1;
            VkDependencyInfo dep{VK_STRUCTURE_TYPE_DEPENDENCY_INFO}; dep.imageMemoryBarrierCount=1; dep.pImageMemoryBarriers=&b;
            vkCmdPipelineBarrier2(cmd,&dep);

            vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,prePipe);
            vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,computePL,0,1,&computeSet,0,nullptr);
            PushPrefilter pcData{rough,face};
            vkCmdPushConstants(cmd,computePL,VK_SHADER_STAGE_COMPUTE_BIT,0,sizeof(PushPrefilter),&pcData);
            vkCmdDispatch(cmd,groups,groups,1);

            vkDestroyImageView(device,faceView,nullptr);
        }
    }

    VkImageMemoryBarrier2 bEnd2{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
    bEnd2.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT; bEnd2.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
    bEnd2.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    bEnd2.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    bEnd2.oldLayout = VK_IMAGE_LAYOUT_GENERAL; bEnd2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    bEnd2.image = outMaps.prefilterImage;
    bEnd2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bEnd2.subresourceRange.levelCount = outMaps.prefilterMips; bEnd2.subresourceRange.layerCount = 6;
    VkDependencyInfo depEnd2{VK_STRUCTURE_TYPE_DEPENDENCY_INFO}; depEnd2.imageMemoryBarrierCount=1; depEnd2.pImageMemoryBarriers=&bEnd2;
    vkCmdPipelineBarrier2(cmd,&depEnd2);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO}; si.commandBufferCount = 1; si.pCommandBuffers = &cmd;
    vkQueueSubmit(queue,1,&si,VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    // Cleanup compute resources
    vkFreeCommandBuffers(device,cmdPool,1,&cmd);
    vkDestroyPipeline(device,prePipe,nullptr);
    vkDestroyPipeline(device,irrPipe,nullptr);
    vkDestroyShaderModule(device,preCS,nullptr);
    vkDestroyShaderModule(device,irrCS,nullptr);
    vkDestroyDescriptorPool(device,computePool,nullptr);
    vkDestroyPipelineLayout(device,computePL,nullptr);
    vkDestroyDescriptorSetLayout(device,computeLayout,nullptr);

    // Descriptor set for PBR pass (irradiance + prefilter)
    VkDescriptorSetLayoutBinding pbrBindings[2]{};
    pbrBindings[0].binding = 0; pbrBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pbrBindings[0].descriptorCount = 1; pbrBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pbrBindings[1].binding = 1; pbrBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pbrBindings[1].descriptorCount = 1; pbrBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    lci.bindingCount = 2; lci.pBindings = pbrBindings;
    if(vkCreateDescriptorSetLayout(device,&lci,nullptr,&outDesc.layout)!=VK_SUCCESS) return false;
    VkDescriptorPoolSize pbrPoolSizes[2]{{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2}};
    dpci.maxSets = 1; dpci.poolSizeCount = 1; dpci.pPoolSizes = pbrPoolSizes; // 2 samplers total
    if(vkCreateDescriptorPool(device,&dpci,nullptr,&outDesc.pool)!=VK_SUCCESS) return false;
    dsai.descriptorPool = outDesc.pool; dsai.descriptorSetCount = 1; dsai.pSetLayouts = &outDesc.layout;
    if(vkAllocateDescriptorSets(device,&dsai,&outDesc.set)!=VK_SUCCESS) return false;

    VkDescriptorImageInfo irrInfo{}; irrInfo.imageView = outMaps.irradianceView; irrInfo.sampler = outMaps.irradianceSampler; irrInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkDescriptorImageInfo preInfo{}; preInfo.imageView = outMaps.prefilterView; preInfo.sampler = outMaps.prefilterSampler; preInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet writes[2]{};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; writes[0].dstSet = outDesc.set; writes[0].dstBinding = 0; writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; writes[0].descriptorCount = 1; writes[0].pImageInfo = &irrInfo;
    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; writes[1].dstSet = outDesc.set; writes[1].dstBinding = 1; writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; writes[1].descriptorCount = 1; writes[1].pImageInfo = &preInfo;
    vkUpdateDescriptorSets(device,2,writes,0,nullptr);

    return true;
}

void DestroyIBLMaps(VkDevice device, VmaAllocator allocator, IBLMaps& maps, IBLDescriptor& desc){
    if(desc.set){}
    if(desc.pool) vkDestroyDescriptorPool(device,desc.pool,nullptr);
    if(desc.layout) vkDestroyDescriptorSetLayout(device,desc.layout,nullptr);
    desc = IBLDescriptor{};
    if(maps.irradianceSampler) vkDestroySampler(device,maps.irradianceSampler,nullptr);
    if(maps.irradianceView) vkDestroyImageView(device,maps.irradianceView,nullptr);
    if(maps.irradianceImage) vmaDestroyImage(allocator,maps.irradianceImage,nullptr);
    if(maps.prefilterSampler) vkDestroySampler(device,maps.prefilterSampler,nullptr);
    if(maps.prefilterView) vkDestroyImageView(device,maps.prefilterView,nullptr);
    if(maps.prefilterImage) vmaDestroyImage(allocator,maps.prefilterImage,nullptr);
    maps = IBLMaps{};
}

} // namespace voxelvk

