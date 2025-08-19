#pragma once
#include <vulkan/vulkan.h>

namespace vkx {

inline void transition_image(VkCommandBuffer cmd,
                             VkImage image,
                             VkImageLayout oldLayout,
                             VkImageLayout newLayout,
                             VkAccessFlags srcAccess,
                             VkAccessFlags dstAccess,
                             VkPipelineStageFlags srcStage,
                             VkPipelineStageFlags dstStage,
                             VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                             uint32_t baseMip = 0, uint32_t levelCount = 1,
                             uint32_t baseLayer = 0, uint32_t layerCount = 1) {
    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.baseMipLevel = baseMip;
    barrier.subresourceRange.levelCount = levelCount;
    barrier.subresourceRange.baseArrayLayer = baseLayer;
    barrier.subresourceRange.layerCount = layerCount;

    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

inline void transition_buffer(VkCommandBuffer cmd,
                              VkBuffer buffer,
                              VkAccessFlags srcAccess,
                              VkAccessFlags dstAccess,
                              VkPipelineStageFlags srcStage,
                              VkPipelineStageFlags dstStage,
                              VkDeviceSize offset = 0,
                              VkDeviceSize size = VK_WHOLE_SIZE) {
    VkBufferMemoryBarrier barrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = buffer;
    barrier.offset = offset;
    barrier.size = size;

    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 1, &barrier, 0, nullptr);
}

inline void memory_barrier(VkCommandBuffer cmd,
                           VkAccessFlags srcAccess,
                           VkAccessFlags dstAccess,
                           VkPipelineStageFlags srcStage,
                           VkPipelineStageFlags dstStage) {
    VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;
    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}

inline void transfer_image_ownership(VkCommandBuffer cmd,
                                     VkImage image,
                                     VkImageLayout layout,
                                     VkAccessFlags srcAccess,
                                     VkAccessFlags dstAccess,
                                     VkPipelineStageFlags srcStage,
                                     VkPipelineStageFlags dstStage,
                                     uint32_t srcQueue,
                                     uint32_t dstQueue,
                                     VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                     uint32_t baseMip = 0, uint32_t levelCount = 1,
                                     uint32_t baseLayer = 0, uint32_t layerCount = 1) {
    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;
    barrier.oldLayout = layout;
    barrier.newLayout = layout;
    barrier.srcQueueFamilyIndex = srcQueue;
    barrier.dstQueueFamilyIndex = dstQueue;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.baseMipLevel = baseMip;
    barrier.subresourceRange.levelCount = levelCount;
    barrier.subresourceRange.baseArrayLayer = baseLayer;
    barrier.subresourceRange.layerCount = layerCount;

    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

inline void transfer_buffer_ownership(VkCommandBuffer cmd,
                                      VkBuffer buffer,
                                      VkAccessFlags srcAccess,
                                      VkAccessFlags dstAccess,
                                      VkPipelineStageFlags srcStage,
                                      VkPipelineStageFlags dstStage,
                                      uint32_t srcQueue,
                                      uint32_t dstQueue,
                                      VkDeviceSize offset = 0,
                                      VkDeviceSize size = VK_WHOLE_SIZE) {
    VkBufferMemoryBarrier barrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;
    barrier.srcQueueFamilyIndex = srcQueue;
    barrier.dstQueueFamilyIndex = dstQueue;
    barrier.buffer = buffer;
    barrier.offset = offset;
    barrier.size = size;

    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 1, &barrier, 0, nullptr);
}

inline VkAccessFlags access_for_layout(VkImageLayout layout) {
    switch(layout){
        case VK_IMAGE_LAYOUT_GENERAL: return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return VK_ACCESS_TRANSFER_WRITE_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return VK_ACCESS_TRANSFER_READ_BIT;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_ACCESS_SHADER_READ_BIT;
        default: return 0;
    }
}

} // namespace vkx
