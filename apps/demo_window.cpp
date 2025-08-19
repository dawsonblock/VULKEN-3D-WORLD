#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <cstring>
#include "vk/swapchain.hpp"

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
    VkQueue queue; vkGetDeviceQueue(device,graphicsQueue,0,&queue);

    // Swapchain helper
    vk::Swapchain swapchain(gpu, device, window, surface);

    VkAttachmentDescription color{}; color.format = swapchain.getFormat(); color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference ref{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription sub{}; sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; sub.colorAttachmentCount=1; sub.pColorAttachments=&ref;
    VkRenderPassCreateInfo rpci{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO}; rpci.attachmentCount=1; rpci.pAttachments=&color; rpci.subpassCount=1; rpci.pSubpasses=&sub;
    VkRenderPass rp; vkCreateRenderPass(device,&rpci,nullptr,&rp);

    std::vector<VkFramebuffer> fbs;
    VkCommandPoolCreateInfo cp{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO}; cp.queueFamilyIndex = graphicsQueue;
    VkCommandPool pool; vkCreateCommandPool(device,&cp,nullptr,&pool);
    std::vector<VkCommandBuffer> cmds;

    auto rebuildFrameResources = [&](){
        for(auto fb : fbs) vkDestroyFramebuffer(device, fb, nullptr);
        if(!cmds.empty()) vkFreeCommandBuffers(device, pool, (uint32_t)cmds.size(), cmds.data());

        uint32_t count = (uint32_t)swapchain.imageCount();
        fbs.resize(count);
        cmds.resize(count);

        for(uint32_t i=0;i<count;i++){
            VkFramebufferCreateInfo fb{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            fb.renderPass = rp; fb.attachmentCount=1; VkImageView view = swapchain.getImageViews()[i]; fb.pAttachments=&view;
            fb.width = swapchain.getExtent().width; fb.height = swapchain.getExtent().height; fb.layers = 1;
            vkCreateFramebuffer(device,&fb,nullptr,&fbs[i]);
        }

        VkCommandBufferAllocateInfo cbai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        cbai.commandPool=pool; cbai.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY; cbai.commandBufferCount=count;
        vkAllocateCommandBuffers(device,&cbai,cmds.data());
        for(uint32_t i=0;i<count;i++){
            VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO}; vkBeginCommandBuffer(cmds[i],&bi);
            VkClearValue clear; clear.color = { {0.1f, 0.2f, 0.3f, 1.0f} };
            VkRenderPassBeginInfo rbi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            rbi.renderPass = rp; rbi.framebuffer = fbs[i]; rbi.renderArea.extent = swapchain.getExtent();
            rbi.clearValueCount = 1; rbi.pClearValues = &clear;
            vkCmdBeginRenderPass(cmds[i],&rbi,VK_SUBPASS_CONTENTS_INLINE);
            vkCmdEndRenderPass(cmds[i]);
            vkEndCommandBuffer(cmds[i]);
        }
    };

    rebuildFrameResources();

    VkSemaphoreCreateInfo sci2{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkSemaphore imgAvailable, renderFinished;
    vkCreateSemaphore(device,&sci2,nullptr,&imgAvailable);
    vkCreateSemaphore(device,&sci2,nullptr,&renderFinished);

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        uint32_t imgIndex; VkResult res = swapchain.acquireNextImage(imgAvailable,&imgIndex);
        if(res == VK_ERROR_OUT_OF_DATE_KHR){ swapchain.recreate(); rebuildFrameResources(); continue; }
        if(res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR){ std::cerr << "acquire failed\n"; break; }
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        si.waitSemaphoreCount=1; si.pWaitSemaphores=&imgAvailable; si.pWaitDstStageMask=&waitStage;
        si.commandBufferCount=1; si.pCommandBuffers=&cmds[imgIndex];
        si.signalSemaphoreCount=1; si.pSignalSemaphores=&renderFinished;
        vkQueueSubmit(queue,1,&si,VK_NULL_HANDLE);
        VkResult pres = swapchain.present(queue, renderFinished, imgIndex);
        if(pres == VK_ERROR_OUT_OF_DATE_KHR || pres == VK_SUBOPTIMAL_KHR || swapchain.framebufferResized){
            swapchain.framebufferResized = false;
            swapchain.recreate();
            rebuildFrameResources();
        } else if(pres != VK_SUCCESS){
            std::cerr << "present failed\n"; break;
        }
        vkQueueWaitIdle(queue);
    }

    vkDeviceWaitIdle(device);
    vkDestroySemaphore(device,renderFinished,nullptr);
    vkDestroySemaphore(device,imgAvailable,nullptr);
    if(!cmds.empty()) vkFreeCommandBuffers(device,pool,(uint32_t)cmds.size(),cmds.data());
    for(auto fb:fbs) vkDestroyFramebuffer(device,fb,nullptr);
    vkDestroyCommandPool(device,pool,nullptr);
    vkDestroyRenderPass(device,rp,nullptr);
    vkDestroyDevice(device,nullptr);
    vkDestroySurfaceKHR(instance,surface,nullptr);
    vkDestroyInstance(instance,nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
