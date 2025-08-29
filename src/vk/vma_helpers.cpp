#include "vma_helpers.hpp"

namespace voxelvk {

bool CreateVmaAllocator(VkInstance instance, VkPhysicalDevice phys, VkDevice device,
                        uint32_t apiVersion, VmaBundle& out)
{
    VmaAllocatorCreateInfo ci{};
    ci.instance = instance;
    ci.physicalDevice = phys;
    ci.device = device;
    ci.vulkanApiVersion = apiVersion;

    VmaAllocator alloc{};
    if(vmaCreateAllocator(&ci, &alloc) != VK_SUCCESS){
        out.allocator = nullptr;
        return false;
    }
    out.allocator = alloc;
    return true;
}

void DestroyVmaAllocator(VmaBundle& bundle)
{
    if(bundle.allocator){
        vmaDestroyAllocator(bundle.allocator);
        bundle.allocator = nullptr;
    }
}

} // namespace voxelvk