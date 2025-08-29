#include <vulkan/vulkan.h>
#include <cstdio>
#include "vk/vma_helpers.hpp"

static bool CreateInstance(VkInstance& instance){
    VkApplicationInfo ai{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    ai.apiVersion = VK_API_VERSION_1_3;
    VkInstanceCreateInfo ci{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    ci.pApplicationInfo = &ai;
    return vkCreateInstance(&ci, nullptr, &instance) == VK_SUCCESS;
}

static bool PickPhysicalDevice(VkInstance instance, VkPhysicalDevice& phys){
    uint32_t count = 0; vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if(count == 0) return false;
    std::vector<VkPhysicalDevice> devs(count);
    vkEnumeratePhysicalDevices(instance, &count, devs.data());
    phys = devs[0];
    return true;
}

static bool CreateDevice(VkPhysicalDevice phys, VkDevice& device){
    float prio = 1.0f;
    VkDeviceQueueCreateInfo qci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    qci.queueFamilyIndex = 0; qci.queueCount = 1; qci.pQueuePriorities = &prio;
    VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    dci.queueCreateInfoCount = 1; dci.pQueueCreateInfos = &qci;
    return vkCreateDevice(phys, &dci, nullptr, &device) == VK_SUCCESS;
}

int main(){
    VkInstance instance = VK_NULL_HANDLE;
    if(!CreateInstance(instance)){
        std::puts("Failed to create Vulkan instance");
        return 1;
    }
    VkPhysicalDevice phys = VK_NULL_HANDLE;
    if(!PickPhysicalDevice(instance, phys)){
        std::puts("Failed to pick physical device");
        return 2;
    }
    VkDevice device = VK_NULL_HANDLE;
    if(!CreateDevice(phys, device)){
        std::puts("Failed to create device");
        return 3;
    }

    voxelvk::VmaBundle bundle{};
    if(!voxelvk::CreateVmaAllocator(instance, phys, device, VK_API_VERSION_1_3, bundle)){
        std::puts("Failed to create VMA allocator (shim)");
        return 4;
    }
    voxelvk::DestroyVmaAllocator(bundle);

    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
    std::puts("vulkan_bootstrap: OK");
    return 0;
}