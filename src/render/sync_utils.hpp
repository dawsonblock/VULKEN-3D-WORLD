#pragma once
#include <vulkan/vulkan.h>

namespace voxelvk {

// Helper to insert an image memory barrier with optional queue ownership transfer.
void barrierImage(
    VkCommandBuffer cmd,
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkAccessFlags srcAccessMask,
    VkAccessFlags dstAccessMask,
    VkImageAspectFlags aspectMask,
    uint32_t srcQueueFamily = VK_QUEUE_FAMILY_IGNORED,
    uint32_t dstQueueFamily = VK_QUEUE_FAMILY_IGNORED,
    uint32_t baseMipLevel = 0,
    uint32_t levelCount = VK_REMAINING_MIP_LEVELS,
    uint32_t baseArrayLayer = 0,
    uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS);

// Helper to insert a buffer memory barrier with optional queue ownership transfer.
void barrierBuffer(
    VkCommandBuffer cmd,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkDeviceSize size,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkAccessFlags srcAccessMask,
    VkAccessFlags dstAccessMask,
    uint32_t srcQueueFamily = VK_QUEUE_FAMILY_IGNORED,
    uint32_t dstQueueFamily = VK_QUEUE_FAMILY_IGNORED);

} // namespace voxelvk

