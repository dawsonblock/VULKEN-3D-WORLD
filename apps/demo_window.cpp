#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <cstring>
#include "../src/render/resource_manager.hpp"

static bool framebufferResized = false;

int main() {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "demo_window", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int, int){
        framebufferResized = true;
        voxelvk::gResourceManager.onSwapchainResize();
    });

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
    uint32_t graphicsQueue = UINT32_MAX, presentQueue = UINT32_MAX;
    for(auto p : phys){
        uint32_t qCount = 0; vkGetPhysicalDeviceQueueFamilyProperties(p,&qCount,nullptr);
        std::vector<VkQueueFamilyProperties> qProps(qCount); vkGetPhysicalDeviceQueueFamilyProperties(p,&qCount,qProps.data());
        for(uint32_t i=0;i<qCount;i++){
            VkBool32 present = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(p,i,surface,&present);
            if((qProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present){
                graphicsQueue = presentQueue = i; gpu = p; break;
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
    voxelvk::gResourceManager.init(device, nullptr, 2);
    VkQueue queue; vkGetDeviceQueue(device,graphicsQueue,0,&queue);

    // Swapchain
    VkSurfaceCapabilitiesKHR caps; vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu,surface,&caps);
    uint32_t fmtCount=0; vkGetPhysicalDeviceSurfaceFormatsKHR(gpu,surface,&fmtCount,nullptr);
    std::vector<VkSurfaceFormatKHR> formats(fmtCount); vkGetPhysicalDeviceSurfaceFormatsKHR(gpu,surface,&fmtCount,formats.data());
    VkSurfaceFormatKHR fmt = formats[0];
    VkSwapchainCreateInfoKHR sci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    sci.surface = surface;
    sci.minImageCount = caps.minImageCount+1;
    if(caps.maxImageCount>0 && sci.minImageCount>caps.maxImageCount) sci.minImageCount=caps.maxImageCount;
    sci.imageFormat = fmt.format; sci.imageColorSpace = fmt.colorSpace;
    sci.imageExtent = caps.currentExtent;
    sci.imageArrayLayers = 1;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sci.preTransform = caps.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    sci.clipped = VK_TRUE;
    VkSwapchainKHR swapchain;
    if(vkCreateSwapchainKHR(device,&sci,nullptr,&swapchain)!=VK_SUCCESS){ std::cerr<<"swapchain failed\n"; return 7; }
    uint32_t imgCount=0; vkGetSwapchainImagesKHR(device,swapchain,&imgCount,nullptr);
    std::vector<VkImage> images(imgCount); vkGetSwapchainImagesKHR(device,swapchain,&imgCount,images.data());

    std::vector<VkImageView> views(imgCount);
    for(uint32_t i=0;i<imgCount;i++){
        VkImageViewCreateInfo iv{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        iv.image = images[i]; iv.viewType = VK_IMAGE_VIEW_TYPE_2D; iv.format = fmt.format;
        iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; iv.subresourceRange.levelCount=1; iv.subresourceRange.layerCount=1;
        vkCreateImageView(device,&iv,nullptr,&views[i]);
    }

    VkAttachmentDescription color{}; color.format = fmt.format; color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference ref{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription sub{}; sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; sub.colorAttachmentCount=1; sub.pColorAttachments=&ref;
    VkRenderPassCreateInfo rpci{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO}; rpci.attachmentCount=1; rpci.pAttachments=&color; rpci.subpassCount=1; rpci.pSubpasses=&sub;
    VkRenderPass rp; vkCreateRenderPass(device,&rpci,nullptr,&rp);

    std::vector<VkFramebuffer> fbs(imgCount);
    for(uint32_t i=0;i<imgCount;i++){
        VkFramebufferCreateInfo fb{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        fb.renderPass = rp; fb.attachmentCount=1; fb.pAttachments=&views[i];
        fb.width = sci.imageExtent.width; fb.height = sci.imageExtent.height; fb.layers = 1;
        vkCreateFramebuffer(device,&fb,nullptr,&fbs[i]);
    }

    VkCommandPoolCreateInfo cp{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO}; cp.queueFamilyIndex = graphicsQueue;
    VkCommandPool pool; vkCreateCommandPool(device,&cp,nullptr,&pool);
    VkCommandBufferAllocateInfo cbai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO}; cbai.commandPool=pool; cbai.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY; cbai.commandBufferCount=imgCount;
    std::vector<VkCommandBuffer> cmds(imgCount); vkAllocateCommandBuffers(device,&cbai,cmds.data());
    for(uint32_t i=0;i<imgCount;i++){
        VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO}; vkBeginCommandBuffer(cmds[i],&bi);
        VkClearValue clear; clear.color = { {0.1f, 0.2f, 0.3f, 1.0f} };
        VkRenderPassBeginInfo rbi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        rbi.renderPass = rp; rbi.framebuffer = fbs[i]; rbi.renderArea.extent = sci.imageExtent;
        rbi.clearValueCount = 1; rbi.pClearValues = &clear;
        vkCmdBeginRenderPass(cmds[i],&rbi,VK_SUBPASS_CONTENTS_INLINE);
        vkCmdEndRenderPass(cmds[i]);
        vkEndCommandBuffer(cmds[i]);
    }

    VkSemaphoreCreateInfo sci2{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkSemaphore imgAvailable, renderFinished;
    vkCreateSemaphore(device,&sci2,nullptr,&imgAvailable);
    vkCreateSemaphore(device,&sci2,nullptr,&renderFinished);

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        if(framebufferResized){
            framebufferResized = false;
            // In a real engine we would recreate swapchain resources here
        }
        uint32_t imgIndex; vkAcquireNextImageKHR(device,swapchain,UINT64_MAX,imgAvailable,VK_NULL_HANDLE,&imgIndex);
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        si.waitSemaphoreCount=1; si.pWaitSemaphores=&imgAvailable; si.pWaitDstStageMask=&waitStage;
        si.commandBufferCount=1; si.pCommandBuffers=&cmds[imgIndex];
        si.signalSemaphoreCount=1; si.pSignalSemaphores=&renderFinished;
        vkQueueSubmit(queue,1,&si,VK_NULL_HANDLE);
        VkPresentInfoKHR pi{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        pi.waitSemaphoreCount=1; pi.pWaitSemaphores=&renderFinished;
        pi.swapchainCount=1; pi.pSwapchains=&swapchain; pi.pImageIndices=&imgIndex;
        vkQueuePresentKHR(queue,&pi);
        vkQueueWaitIdle(queue);
    }

    vkDeviceWaitIdle(device);
    vkDestroySemaphore(device,renderFinished,nullptr);
    vkDestroySemaphore(device,imgAvailable,nullptr);
    vkDestroyCommandPool(device,pool,nullptr);
    for(auto fb:fbs) vkDestroyFramebuffer(device,fb,nullptr);
    vkDestroyRenderPass(device,rp,nullptr);
    for(auto v:views) vkDestroyImageView(device,v,nullptr);
    vkDestroySwapchainKHR(device,swapchain,nullptr);
    voxelvk::gResourceManager.shutdown();
    vkDestroyDevice(device,nullptr);
    vkDestroySurfaceKHR(instance,surface,nullptr);
    vkDestroyInstance(instance,nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
