#include "swapchain.hpp"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <stdexcept>

namespace vk {

static void framebufferResizeCallback(GLFWwindow* window, int, int) {
    auto sc = reinterpret_cast<Swapchain*>(glfwGetWindowUserPointer(window));
    if(sc) sc->framebufferResized = true;
}

Swapchain::Swapchain(VkPhysicalDevice p, VkDevice d, GLFWwindow* w, VkSurfaceKHR s)
    : phys(p), device(d), window(w), surface(s) {
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    recreate();
}

Swapchain::~Swapchain() {
    cleanup();
}

VkSurfaceFormatKHR Swapchain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
    for(const auto& f : formats) {
        if((f.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT ||
            f.colorSpace == VK_COLOR_SPACE_BT2020_LINEAR_EXT) &&
           f.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32) {
            return f;
        }
    }
    for(const auto& f : formats) {
        if(f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return f;
    }
    return formats.empty() ? VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} : formats[0];
}

VkPresentModeKHR Swapchain::choosePresentMode(const std::vector<VkPresentModeKHR>& modes) {
    for(auto m : modes) {
        if(m == VK_PRESENT_MODE_MAILBOX_KHR)
            return m;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

void Swapchain::recreate() {
    int width=0,height=0;
    glfwGetFramebufferSize(window,&width,&height);
    while(width==0 || height==0){
        glfwGetFramebufferSize(window,&width,&height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);
    cleanup();

    VkSurfaceCapabilitiesKHR caps; vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys, surface, &caps);
    uint32_t fmtCount=0; vkGetPhysicalDeviceSurfaceFormatsKHR(phys, surface, &fmtCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(fmtCount); vkGetPhysicalDeviceSurfaceFormatsKHR(phys, surface, &fmtCount, formats.data());
    uint32_t pmCount=0; vkGetPhysicalDeviceSurfacePresentModesKHR(phys, surface, &pmCount, nullptr);
    std::vector<VkPresentModeKHR> modes(pmCount); vkGetPhysicalDeviceSurfacePresentModesKHR(phys, surface, &pmCount, modes.data());

    VkSurfaceFormatKHR surfFmt = chooseSurfaceFormat(formats);
    VkPresentModeKHR present = choosePresentMode(modes);

    if(caps.currentExtent.width != UINT32_MAX) {
        extent = caps.currentExtent;
    } else {
        extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        extent.width = std::clamp(extent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
        extent.height = std::clamp(extent.height, caps.minImageExtent.height, caps.maxImageExtent.height);
    }

    uint32_t imageCount = caps.minImageCount + 1;
    if(caps.maxImageCount > 0 && imageCount > caps.maxImageCount) imageCount = caps.maxImageCount;

    VkSwapchainCreateInfoKHR sci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    sci.surface = surface;
    sci.minImageCount = imageCount;
    sci.imageFormat = surfFmt.format;
    sci.imageColorSpace = surfFmt.colorSpace;
    sci.imageExtent = extent;
    sci.imageArrayLayers = 1;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sci.preTransform = caps.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode = present;
    sci.clipped = VK_TRUE;
    sci.oldSwapchain = swapchain;

    if(vkCreateSwapchainKHR(device, &sci, nullptr, &swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swapchain");
    }

    format = surfFmt.format;

    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());

    imageViews.resize(imageCount);
    for(uint32_t i=0;i<imageCount;i++){
        VkImageViewCreateInfo iv{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        iv.image = images[i];
        iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
        iv.format = format;
        iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        iv.subresourceRange.levelCount = 1;
        iv.subresourceRange.layerCount = 1;
        vkCreateImageView(device, &iv, nullptr, &imageViews[i]);
    }
}

void Swapchain::cleanup() {
    for(auto v : imageViews) {
        vkDestroyImageView(device, v, nullptr);
    }
    imageViews.clear();
    images.clear();
    if(swapchain != VK_NULL_HANDLE){
        vkDestroySwapchainKHR(device, swapchain, nullptr);
        swapchain = VK_NULL_HANDLE;
    }
}

VkResult Swapchain::acquireNextImage(VkSemaphore available, uint32_t* index) {
    return vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, available, VK_NULL_HANDLE, index);
}

VkResult Swapchain::present(VkQueue queue, VkSemaphore wait, uint32_t index) {
    VkPresentInfoKHR pi{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &wait;
    pi.swapchainCount = 1;
    pi.pSwapchains = &swapchain;
    pi.pImageIndices = &index;
    return vkQueuePresentKHR(queue, &pi);
}

}

