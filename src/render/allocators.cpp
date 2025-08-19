#include "allocators.hpp"
#include <cstring>

namespace voxelvk {

bool createStagingBuffer(VmaAllocator allocator, VkDeviceSize size, StagingBuffer& out){
    VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bi.size = size;
    bi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo aci{};
    aci.usage = VMA_MEMORY_USAGE_AUTO;
    aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VmaAllocationInfo info{};
    if(vmaCreateBuffer(allocator, &bi, &aci, &out.buffer, &out.allocation, &info) != VK_SUCCESS)
        return false;
    out.mapped = info.pMappedData;
    return true;
}

void destroyStagingBuffer(VmaAllocator allocator, StagingBuffer& buf){
    if(buf.buffer != VK_NULL_HANDLE){
        vmaDestroyBuffer(allocator, buf.buffer, buf.allocation);
        buf.buffer = VK_NULL_HANDLE;
        buf.allocation = nullptr;
        buf.mapped = nullptr;
    }
}

bool uploadBuffer(VmaAllocator allocator, VkCommandBuffer cmd, VkBuffer dst, const void* data, VkDeviceSize size){
    StagingBuffer staging{};
    if(!createStagingBuffer(allocator, size, staging)) return false;
    std::memcpy(staging.mapped, data, static_cast<size_t>(size));
    VkBufferCopy copy{0,0,size};
    vkCmdCopyBuffer(cmd, staging.buffer, dst, 1, &copy);
    destroyStagingBuffer(allocator, staging);
    return true;
}

bool readbackBuffer(VmaAllocator allocator, VkCommandBuffer cmd, VkBuffer src, void* dst, VkDeviceSize size){
    StagingBuffer staging{};
    if(!createStagingBuffer(allocator, size, staging)) return false;
    VkBufferCopy copy{0,0,size};
    vkCmdCopyBuffer(cmd, src, staging.buffer, 1, &copy);
    std::memcpy(dst, staging.mapped, static_cast<size_t>(size));
    destroyStagingBuffer(allocator, staging);
    return true;
}

bool allocateImage(VmaAllocator allocator, const VkImageCreateInfo& ci, VkImage& image, VmaAllocation& alloc){
    VmaAllocationCreateInfo aci{};
    aci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    return vmaCreateImage(allocator, &ci, &aci, &image, &alloc, nullptr) == VK_SUCCESS;
}

bool allocateDeviceBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& alloc){
    VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bi.size = size;
    bi.usage = usage;
    VmaAllocationCreateInfo aci{};
    aci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    return vmaCreateBuffer(allocator, &bi, &aci, &buffer, &alloc, nullptr) == VK_SUCCESS;
}

void destroyImage(VmaAllocator allocator, VkImage& image, VmaAllocation& alloc){
    if(image != VK_NULL_HANDLE){
        vmaDestroyImage(allocator, image, alloc);
        image = VK_NULL_HANDLE;
        alloc = nullptr;
    }
}

void destroyBuffer(VmaAllocator allocator, VkBuffer& buffer, VmaAllocation& alloc){
    if(buffer != VK_NULL_HANDLE){
        vmaDestroyBuffer(allocator, buffer, alloc);
        buffer = VK_NULL_HANDLE;
        alloc = nullptr;
    }
}

void TransientBufferPool::init(VmaAllocator a){
    allocator = a;
}

void TransientBufferPool::reset(){
    for(auto& b : buffers){
        vmaDestroyBuffer(allocator, b.first, b.second);
    }
    buffers.clear();
}

void TransientBufferPool::destroy(){
    reset();
    allocator = nullptr;
}

bool TransientBufferPool::allocate(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& allocation){
    if(!allocateDeviceBuffer(allocator, size, usage, buffer, allocation))
        return false;
    buffers.emplace_back(buffer, allocation);
    return true;
}

} // namespace voxelvk

