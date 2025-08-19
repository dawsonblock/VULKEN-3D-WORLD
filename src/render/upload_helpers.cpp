#include "upload_helpers.hpp"
#include <cstring>

namespace voxelvk {

bool createStagingBuffer(VkPhysicalDevice phys,
                         VkDevice device,
                         VkDeviceSize size,
                         VkBuffer* outBuffer,
                         VkDeviceMemory* outMemory) {
    if (!outBuffer || !outMemory) return false;

    VkBufferCreateInfo bci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bci.size = size;
    bci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(device, &bci, nullptr, outBuffer) != VK_SUCCESS) {
        return false;
    }

    VkMemoryRequirements req{};
    vkGetBufferMemoryRequirements(device, *outBuffer, &req);

    VkPhysicalDeviceMemoryProperties memProps{};
    vkGetPhysicalDeviceMemoryProperties(phys, &memProps);
    uint32_t memIndex = UINT32_MAX;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((req.memoryTypeBits & (1u << i)) &&
            (memProps.memoryTypes[i].propertyFlags &
             (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) ==
                (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            memIndex = i;
            break;
        }
    }
    if (memIndex == UINT32_MAX) {
        vkDestroyBuffer(device, *outBuffer, nullptr);
        return false;
    }

    VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    mai.allocationSize = req.size;
    mai.memoryTypeIndex = memIndex;
    if (vkAllocateMemory(device, &mai, nullptr, outMemory) != VK_SUCCESS) {
        vkDestroyBuffer(device, *outBuffer, nullptr);
        return false;
    }

    if (vkBindBufferMemory(device, *outBuffer, *outMemory, 0) != VK_SUCCESS) {
        vkDestroyBuffer(device, *outBuffer, nullptr);
        vkFreeMemory(device, *outMemory, nullptr);
        return false;
    }
    return true;
}

} // namespace voxelvk

