#pragma once

#include <vulkan/vulkan.h>
#include <vector>

struct GLFWwindow;

namespace vk {

class Swapchain {
public:
    Swapchain(VkPhysicalDevice phys, VkDevice device, GLFWwindow* window, VkSurfaceKHR surface);
    ~Swapchain();

    VkSwapchainKHR get() const { return swapchain; }
    VkFormat getFormat() const { return format; }
    VkExtent2D getExtent() const { return extent; }
    const std::vector<VkImageView>& getImageViews() const { return imageViews; }
    size_t imageCount() const { return imageViews.size(); }

    VkResult acquireNextImage(VkSemaphore available, uint32_t* index);
    VkResult present(VkQueue queue, VkSemaphore wait, uint32_t index);

    void recreate();
    void cleanup();

    bool framebufferResized = false;

private:
    VkPhysicalDevice phys;
    VkDevice device;
    GLFWwindow* window;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat format{};
    VkExtent2D extent{};
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& modes);
};

}

