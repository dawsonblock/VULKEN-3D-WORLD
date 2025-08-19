#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

struct ResourceManager {
    VkDevice device{};
    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    VkFormat format{};
    VkExtent2D extent{};
    std::vector<VkImage> images;
    std::vector<VkImageView> views;
    VkRenderPass renderPass{VK_NULL_HANDLE};
    std::vector<VkFramebuffer> framebuffers;
    VkCommandPool commandPool{VK_NULL_HANDLE};
    std::vector<VkCommandBuffer> commandBuffers;
    void cleanup() {
        for(auto fb : framebuffers) vkDestroyFramebuffer(device, fb, nullptr);
        for(auto v : views) vkDestroyImageView(device, v, nullptr);
        if(commandPool) vkDestroyCommandPool(device, commandPool, nullptr);
        if(renderPass) vkDestroyRenderPass(device, renderPass, nullptr);
        if(swapchain) vkDestroySwapchainKHR(device, swapchain, nullptr);
        framebuffers.clear(); views.clear(); images.clear(); commandBuffers.clear();
        swapchain = VK_NULL_HANDLE; renderPass = VK_NULL_HANDLE; commandPool = VK_NULL_HANDLE;
    }
};

VkPresentModeKHR choosePresentMode(VkPhysicalDevice gpu, VkSurfaceKHR surface) {
    uint32_t count = 0; vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &count, nullptr);
    std::vector<VkPresentModeKHR> modes(count); vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &count, modes.data());
    for(auto m : modes) if(m == VK_PRESENT_MODE_MAILBOX_KHR) return VK_PRESENT_MODE_MAILBOX_KHR;
    for(auto m : modes) if(m == VK_PRESENT_MODE_FIFO_KHR) return VK_PRESENT_MODE_FIFO_KHR;
    return modes.empty() ? VK_PRESENT_MODE_FIFO_KHR : modes[0];
}

VkSurfaceFormatKHR chooseSurfaceFormat(VkPhysicalDevice gpu, VkSurfaceKHR surface, bool hdr) {
    uint32_t count = 0; vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(count); vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &count, formats.data());
    VkSurfaceFormatKHR fallback = formats[0];
    if(hdr) {
        for(auto& f : formats) {
            if(f.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 && f.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT) return f;
        }
    }
    for(auto& f : formats) {
        if(f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return f;
    }
    return fallback;
}

void createSwapchain(ResourceManager& rm, VkPhysicalDevice gpu, VkSurfaceKHR surface,
                     VkDevice device, uint32_t queueFamily, bool hdr) {
    VkSurfaceCapabilitiesKHR caps; vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &caps);
    VkSurfaceFormatKHR fmt = chooseSurfaceFormat(gpu, surface, hdr);
    VkPresentModeKHR mode = choosePresentMode(gpu, surface);
    VkSwapchainCreateInfoKHR sci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    sci.surface = surface;
    sci.minImageCount = caps.minImageCount + 1;
    if(caps.maxImageCount > 0 && sci.minImageCount > caps.maxImageCount) sci.minImageCount = caps.maxImageCount;
    sci.imageFormat = fmt.format; sci.imageColorSpace = fmt.colorSpace;
    sci.imageExtent = caps.currentExtent;
    sci.imageArrayLayers = 1;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sci.preTransform = caps.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode = mode;
    sci.clipped = VK_TRUE;
    if(vkCreateSwapchainKHR(device, &sci, nullptr, &rm.swapchain) != VK_SUCCESS) {
        throw std::runtime_error("swapchain failed");
    }
    rm.device = device; rm.format = fmt.format; rm.extent = caps.currentExtent;
    uint32_t imgCount = 0; vkGetSwapchainImagesKHR(device, rm.swapchain, &imgCount, nullptr);
    rm.images.resize(imgCount); vkGetSwapchainImagesKHR(device, rm.swapchain, &imgCount, rm.images.data());

    rm.views.resize(imgCount);
    for(uint32_t i=0;i<imgCount;i++){
        VkImageViewCreateInfo iv{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        iv.image = rm.images[i]; iv.viewType = VK_IMAGE_VIEW_TYPE_2D; iv.format = rm.format;
        iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; iv.subresourceRange.levelCount=1; iv.subresourceRange.layerCount=1;
        vkCreateImageView(device,&iv,nullptr,&rm.views[i]);
    }

    VkAttachmentDescription color{}; color.format = rm.format; color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference ref{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription sub{}; sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; sub.colorAttachmentCount=1; sub.pColorAttachments=&ref;
    VkRenderPassCreateInfo rpci{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO}; rpci.attachmentCount=1; rpci.pAttachments=&color; rpci.subpassCount=1; rpci.pSubpasses=&sub;
    vkCreateRenderPass(device,&rpci,nullptr,&rm.renderPass);

    rm.framebuffers.resize(imgCount);
    for(uint32_t i=0;i<imgCount;i++){
        VkFramebufferCreateInfo fb{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        fb.renderPass = rm.renderPass; fb.attachmentCount=1; fb.pAttachments=&rm.views[i];
        fb.width = rm.extent.width; fb.height = rm.extent.height; fb.layers = 1;
        vkCreateFramebuffer(device,&fb,nullptr,&rm.framebuffers[i]);
    }

    VkCommandPoolCreateInfo cp{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO}; cp.queueFamilyIndex = queueFamily;
    vkCreateCommandPool(device,&cp,nullptr,&rm.commandPool);
    VkCommandBufferAllocateInfo cbai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO}; cbai.commandPool=rm.commandPool; cbai.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY; cbai.commandBufferCount=imgCount;
    rm.commandBuffers.resize(imgCount); vkAllocateCommandBuffers(device,&cbai,rm.commandBuffers.data());
    for(uint32_t i=0;i<imgCount;i++){
        VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO}; vkBeginCommandBuffer(rm.commandBuffers[i],&bi);
        VkClearValue clear; clear.color = { {0.1f, 0.2f, 0.3f, 1.0f} };
        VkRenderPassBeginInfo rbi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        rbi.renderPass = rm.renderPass; rbi.framebuffer = rm.framebuffers[i]; rbi.renderArea.extent = rm.extent;
        rbi.clearValueCount = 1; rbi.pClearValues = &clear;
        vkCmdBeginRenderPass(rm.commandBuffers[i],&rbi,VK_SUBPASS_CONTENTS_INLINE);
        vkCmdEndRenderPass(rm.commandBuffers[i]);
        vkEndCommandBuffer(rm.commandBuffers[i]);
    }
}

int main() {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "demo_window", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }

    // Instance
    std::vector<const char*> layers;
#ifndef NDEBUG
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
    uint32_t extCount = 0;
    const char** glfwExt = glfwGetRequiredInstanceExtensions(&extCount);
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = "demo_window";
    app.apiVersion = VK_API_VERSION_1_2;
    VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    ici.pApplicationInfo = &app;
    ici.enabledExtensionCount = extCount;
    ici.ppEnabledExtensionNames = glfwExt;
    ici.enabledLayerCount = (uint32_t)layers.size();
    ici.ppEnabledLayerNames = layers.empty()? nullptr : layers.data();
    VkInstance instance;
    if(vkCreateInstance(&ici,nullptr,&instance)!=VK_SUCCESS){
        std::cerr << "vkCreateInstance failed\n"; return 2;
    }

    // Surface
    VkSurfaceKHR surface;
    if(glfwCreateWindowSurface(instance,window,nullptr,&surface)!=VK_SUCCESS){
        std::cerr << "create surface failed\n"; return 3;
    }

    // Physical device selection
    uint32_t pdCount = 0; vkEnumeratePhysicalDevices(instance,&pdCount,nullptr);
    if(pdCount==0){ std::cerr << "no device\n"; return 4; }
    std::vector<VkPhysicalDevice> phys(pdCount); vkEnumeratePhysicalDevices(instance,&pdCount,phys.data());
    VkPhysicalDevice gpu = VK_NULL_HANDLE;
    uint32_t graphicsQueue = UINT32_MAX;
    for(auto p : phys){
        uint32_t qCount = 0; vkGetPhysicalDeviceQueueFamilyProperties(p,&qCount,nullptr);
        std::vector<VkQueueFamilyProperties> qProps(qCount); vkGetPhysicalDeviceQueueFamilyProperties(p,&qCount,qProps.data());
        for(uint32_t i=0;i<qCount;i++){
            VkBool32 present = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(p,i,surface,&present);
            if((qProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present){
                graphicsQueue = i; gpu = p; break;
            }
        }
        if(gpu!=VK_NULL_HANDLE) break;
    }
    if(gpu==VK_NULL_HANDLE){ std::cerr << "no suitable device\n"; return 5; }

    float prio = 1.0f;
    VkDeviceQueueCreateInfo dq{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    dq.queueFamilyIndex = graphicsQueue; dq.queueCount = 1; dq.pQueuePriorities = &prio;
    const char* devExts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    dci.queueCreateInfoCount = 1; dci.pQueueCreateInfos = &dq;
    dci.enabledExtensionCount = 1; dci.ppEnabledExtensionNames = devExts;
    VkDevice device;
    if(vkCreateDevice(gpu,&dci,nullptr,&device)!=VK_SUCCESS){ std::cerr<<"vkCreateDevice failed\n"; return 6; }
    VkQueue queue; vkGetDeviceQueue(device,graphicsQueue,0,&queue);

    ResourceManager rm{};
    bool hdr = std::getenv("ENABLE_HDR") != nullptr;
    createSwapchain(rm, gpu, surface, device, graphicsQueue, hdr);

    VkSemaphoreCreateInfo sci2{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkSemaphore imgAvailable, renderFinished;
    vkCreateSemaphore(device,&sci2,nullptr,&imgAvailable);
    vkCreateSemaphore(device,&sci2,nullptr,&renderFinished);

    auto recreate = [&](){
        vkDeviceWaitIdle(device);
        rm.cleanup();
        createSwapchain(rm, gpu, surface, device, graphicsQueue, hdr);
    };

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        uint32_t imgIndex;
        VkResult ar = vkAcquireNextImageKHR(device,rm.swapchain,UINT64_MAX,imgAvailable,VK_NULL_HANDLE,&imgIndex);
        if(ar == VK_ERROR_OUT_OF_DATE_KHR){
            recreate();
            continue;
        }
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        si.waitSemaphoreCount=1; si.pWaitSemaphores=&imgAvailable; si.pWaitDstStageMask=&waitStage;
        si.commandBufferCount=1; si.pCommandBuffers=&rm.commandBuffers[imgIndex];
        si.signalSemaphoreCount=1; si.pSignalSemaphores=&renderFinished;
        vkQueueSubmit(queue,1,&si,VK_NULL_HANDLE);
        VkPresentInfoKHR pi{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        pi.waitSemaphoreCount=1; pi.pWaitSemaphores=&renderFinished;
        pi.swapchainCount=1; pi.pSwapchains=&rm.swapchain; pi.pImageIndices=&imgIndex;
        VkResult pr = vkQueuePresentKHR(queue,&pi);
        if(pr == VK_ERROR_OUT_OF_DATE_KHR || pr == VK_SUBOPTIMAL_KHR){
            recreate();
        }
        vkQueueWaitIdle(queue);
    }

    vkDeviceWaitIdle(device);
    vkDestroySemaphore(device,renderFinished,nullptr);
    vkDestroySemaphore(device,imgAvailable,nullptr);
    rm.cleanup();
    vkDestroyDevice(device,nullptr);
    vkDestroySurfaceKHR(instance,surface,nullptr);
    vkDestroyInstance(instance,nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
