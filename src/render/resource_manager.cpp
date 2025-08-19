#include "resource_manager.hpp"
#include "vk_mem_alloc.h"

namespace voxelvk {

void ResourceManager::init(VkDevice dev, VmaAllocator alloc, uint32_t frames){
    device = dev;
    allocator = alloc;
    frameCount = frames;
    currentFrame = 0;
    frameAllocs.resize(frames);
}

void ResourceManager::shutdown(){
    for(auto& f : frameAllocs){
        for(size_t i=0;i<f.buffers.size();++i){
            if(f.buffers[i]){
                vmaDestroyBuffer(allocator, f.buffers[i], f.allocs[i]);
            }
        }
        f.buffers.clear();
        f.allocs.clear();
    }
    frameAllocs.clear();
    globalBuffers.clear();
    globalImages.clear();
    device = VK_NULL_HANDLE;
    allocator = nullptr;
}

void ResourceManager::beginFrame(uint32_t frameIndex){
    currentFrame = frameIndex % frameCount;
    auto& f = frameAllocs[currentFrame];
    for(size_t i=0;i<f.buffers.size();++i){
        if(f.buffers[i]){
            vmaDestroyBuffer(allocator, f.buffers[i], f.allocs[i]);
        }
    }
    f.buffers.clear();
    f.allocs.clear();
}

void ResourceManager::onSwapchainResize(){
    for(auto& f : frameAllocs){
        for(size_t i=0;i<f.buffers.size();++i){
            if(f.buffers[i]){
                vmaDestroyBuffer(allocator, f.buffers[i], f.allocs[i]);
            }
        }
        f.buffers.clear();
        f.allocs.clear();
    }
}

VkResult ResourceManager::createShaderModule(const VkShaderModuleCreateInfo* ci, VkShaderModule* out) const{
    return vkCreateShaderModule(device, ci, nullptr, out);
}

VkResult ResourceManager::createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* ci, VkDescriptorSetLayout* out) const{
    return vkCreateDescriptorSetLayout(device, ci, nullptr, out);
}

VkResult ResourceManager::createDescriptorPool(const VkDescriptorPoolCreateInfo* ci, VkDescriptorPool* out) const{
    return vkCreateDescriptorPool(device, ci, nullptr, out);
}

VkResult ResourceManager::createPipelineLayout(const VkPipelineLayoutCreateInfo* ci, VkPipelineLayout* out) const{
    return vkCreatePipelineLayout(device, ci, nullptr, out);
}

VkResult ResourceManager::createGraphicsPipeline(const VkGraphicsPipelineCreateInfo* ci, VkPipeline* out) const{
    return vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, ci, nullptr, out);
}

VkResult ResourceManager::createComputePipeline(const VkComputePipelineCreateInfo* ci, VkPipeline* out) const{
    return vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, ci, nullptr, out);
}

VkResult ResourceManager::createImageView(const VkImageViewCreateInfo* ci, VkImageView* out) const{
    return vkCreateImageView(device, ci, nullptr, out);
}

VkResult ResourceManager::createSampler(const VkSamplerCreateInfo* ci, VkSampler* out) const{
    return vkCreateSampler(device, ci, nullptr, out);
}

VkBuffer ResourceManager::createStagingBuffer(VkDeviceSize size, void** mapped, VmaAllocation* outAlloc){
    VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bi.size = size;
    bi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VmaAllocationCreateInfo aci{};
    aci.usage = VMA_MEMORY_USAGE_AUTO;
    aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
               VMA_ALLOCATION_CREATE_MAPPED_BIT;
    VmaAllocationInfo info{};
    VkBuffer buf;
    if(vmaCreateBuffer(allocator, &bi, &aci, &buf, outAlloc, &info) != VK_SUCCESS)
        return VK_NULL_HANDLE;
    if(mapped) *mapped = info.pMappedData;
    auto& f = frameAllocs[currentFrame];
    f.buffers.push_back(buf);
    f.allocs.push_back(*outAlloc);
    return buf;
}

void ResourceManager::destroyStagingBuffer(VkBuffer buf, VmaAllocation alloc){
    if(buf) vmaDestroyBuffer(allocator, buf, alloc);
}

void ResourceManager::registerBuffer(const std::string& name, VkBuffer buf){
    globalBuffers[name] = buf;
}

VkBuffer ResourceManager::getBuffer(const std::string& name) const{
    auto it = globalBuffers.find(name);
    return it == globalBuffers.end() ? VK_NULL_HANDLE : it->second;
}

void ResourceManager::registerImage(const std::string& name, VkImage img){
    globalImages[name] = img;
}

VkImage ResourceManager::getImage(const std::string& name) const{
    auto it = globalImages.find(name);
    return it == globalImages.end() ? VK_NULL_HANDLE : it->second;
}

ResourceManager gResourceManager;

} // namespace voxelvk

