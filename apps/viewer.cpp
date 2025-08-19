#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <vector>

#if defined(ENABLE_IMGUI_OVERLAY) && ENABLE_IMGUI_OVERLAY
# if __has_include("imgui.h")
#  include "imgui.h"
#  include "imgui_impl_glfw.h"
#  include "imgui_impl_vulkan.h"
#  define VOXELVK_HAS_IMGUI 1
# endif
#endif
#ifndef VOXELVK_HAS_IMGUI
# define VOXELVK_HAS_IMGUI 0
#endif

struct Camera {
    float x{0.f}, y{0.f}, z{5.f};
    float yaw{-90.f};
    float pitch{0.f};
    float speed{5.f};
    float sensitivity{0.1f};
    bool firstMouse{true};
    double lastX{0.0}, lastY{0.0};

    void handleCursor(GLFWwindow* window, double xpos, double ypos) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
        float xoffset=float(xpos-lastX); float yoffset=float(lastY-ypos);
        lastX=xpos; lastY=ypos;
        yaw += xoffset*sensitivity; pitch += yoffset*sensitivity;
        if(pitch>89.f) pitch=89.f; if(pitch<-89.f) pitch=-89.f;
    }

    void update(GLFWwindow* window, float dt){
        float vx = std::cos(glm_rad(yaw))*std::cos(glm_rad(pitch));
        float vy = std::sin(glm_rad(pitch));
        float vz = std::sin(glm_rad(yaw))*std::cos(glm_rad(pitch));
        float frontX=vx, frontY=vy, frontZ=vz;
        float rightX=-frontZ, rightY=0, rightZ=frontX;
        float upX=0, upY=1, upZ=0;
        if(glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS){ x+=frontX*speed*dt; y+=frontY*speed*dt; z+=frontZ*speed*dt; }
        if(glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS){ x-=frontX*speed*dt; y-=frontY*speed*dt; z-=frontZ*speed*dt; }
        if(glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS){ x-=rightX*speed*dt; y-=rightY*speed*dt; z-=rightZ*speed*dt; }
        if(glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS){ x+=rightX*speed*dt; y+=rightY*speed*dt; z+=rightZ*speed*dt; }
        if(glfwGetKey(window,GLFW_KEY_SPACE)==GLFW_PRESS){ y+=upY*speed*dt; }
        if(glfwGetKey(window,GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS){ y-=upY*speed*dt; }
    }
};

static float glm_rad(float deg){ return deg * static_cast<float>(M_PI) / 180.f; }

struct HotReloadShader {
    std::filesystem::path path;
    std::filesystem::file_time_type timestamp{};
    VkDevice device{};
    VkPipeline pipeline{VK_NULL_HANDLE};

    HotReloadShader(VkDevice d, const std::filesystem::path& p):path(p),device(d){
        if(std::filesystem::exists(path)) timestamp = std::filesystem::last_write_time(path);
    }

    void reload(){
        // Placeholder: shader compilation should occur here.
        std::cout << "Reloading shader: " << path << std::endl;
    }

    void update(){
        if(!std::filesystem::exists(path)) return;
        auto ts = std::filesystem::last_write_time(path);
        if(ts != timestamp){
            timestamp = ts;
            reload();
        }
    }
};

struct DebugLabel {
    PFN_vkCmdBeginDebugUtilsLabelEXT begin{nullptr};
    PFN_vkCmdEndDebugUtilsLabelEXT end{nullptr};

    void init(VkInstance instance){
        begin = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance,"vkCmdBeginDebugUtilsLabelEXT"));
        end   = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance,"vkCmdEndDebugUtilsLabelEXT"));
    }

    void beginLabel(VkCommandBuffer cmd, const char* name){
        if(!begin) return;
        VkDebugUtilsLabelEXT label{VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT};
        label.pLabelName = name;
        begin(cmd,&label);
    }
    void endLabel(VkCommandBuffer cmd){ if(end) end(cmd); }
};

int main(){
    if(!glfwInit()) return 1;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280,720,"pbr_viewer",nullptr,nullptr);
    if(!window){ glfwTerminate(); return 1; }

    Camera camera{};
    glfwSetWindowUserPointer(window,&camera);
    glfwSetCursorPosCallback(window,[](GLFWwindow* win,double x,double y){
        static_cast<Camera*>(glfwGetWindowUserPointer(win))->handleCursor(win,x,y);
    });
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Vulkan instance
    std::vector<const char*> layers;
#ifndef NDEBUG
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
    uint32_t extCount = 0;
    const char** glfwExt = glfwGetRequiredInstanceExtensions(&extCount);
    std::vector<const char*> extensions(glfwExt, glfwExt+extCount);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = "pbr_viewer";
    app.apiVersion = VK_API_VERSION_1_2;
    VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    ici.pApplicationInfo = &app;
    ici.enabledExtensionCount = (uint32_t)extensions.size();
    ici.ppEnabledExtensionNames = extensions.data();
    ici.enabledLayerCount = (uint32_t)layers.size();
    ici.ppEnabledLayerNames = layers.empty()? nullptr : layers.data();
    VkInstance instance; if(vkCreateInstance(&ici,nullptr,&instance)!=VK_SUCCESS){
        std::cerr << "vkCreateInstance failed\n"; return 2; }

    DebugLabel dbg; dbg.init(instance);

    VkSurfaceKHR surface; glfwCreateWindowSurface(instance,window,nullptr,&surface);

    // Physical device selection
    uint32_t pdCount=0; vkEnumeratePhysicalDevices(instance,&pdCount,nullptr);
    std::vector<VkPhysicalDevice> phys(pdCount); vkEnumeratePhysicalDevices(instance,&pdCount,phys.data());
    VkPhysicalDevice gpu=phys[0];
    uint32_t qCount=0; vkGetPhysicalDeviceQueueFamilyProperties(gpu,&qCount,nullptr);
    std::vector<VkQueueFamilyProperties> qProps(qCount); vkGetPhysicalDeviceQueueFamilyProperties(gpu,&qCount,qProps.data());
    uint32_t graphicsQueue=0; for(uint32_t i=0;i<qCount;i++){ VkBool32 present=VK_FALSE; vkGetPhysicalDeviceSurfaceSupportKHR(gpu,i,surface,&present); if((qProps[i].queueFlags&VK_QUEUE_GRAPHICS_BIT)&&present){graphicsQueue=i;break;} }

    float prio=1.f; VkDeviceQueueCreateInfo dq{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO}; dq.queueFamilyIndex=graphicsQueue; dq.queueCount=1; dq.pQueuePriorities=&prio;
    const char* devExts[]={VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO}; dci.queueCreateInfoCount=1; dci.pQueueCreateInfos=&dq; dci.enabledExtensionCount=1; dci.ppEnabledExtensionNames=devExts;
    VkDevice device; vkCreateDevice(gpu,&dci,nullptr,&device);
    VkQueue queue; vkGetDeviceQueue(device,graphicsQueue,0,&queue);

    VkSurfaceCapabilitiesKHR caps; vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu,surface,&caps);
    uint32_t fmtCount=0; vkGetPhysicalDeviceSurfaceFormatsKHR(gpu,surface,&fmtCount,nullptr);
    std::vector<VkSurfaceFormatKHR> formats(fmtCount); vkGetPhysicalDeviceSurfaceFormatsKHR(gpu,surface,&fmtCount,formats.data());
    VkSurfaceFormatKHR fmt=formats[0];
    VkSwapchainCreateInfoKHR sci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    sci.surface=surface; sci.minImageCount=caps.minImageCount+1; if(caps.maxImageCount>0 && sci.minImageCount>caps.maxImageCount) sci.minImageCount=caps.maxImageCount; sci.imageFormat=fmt.format; sci.imageColorSpace=fmt.colorSpace; sci.imageExtent=caps.currentExtent; sci.imageArrayLayers=1; sci.imageUsage=VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; sci.imageSharingMode=VK_SHARING_MODE_EXCLUSIVE; sci.preTransform=caps.currentTransform; sci.compositeAlpha=VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; sci.presentMode=VK_PRESENT_MODE_FIFO_KHR; sci.clipped=VK_TRUE;
    VkSwapchainKHR swapchain; vkCreateSwapchainKHR(device,&sci,nullptr,&swapchain);
    uint32_t imgCount=0; vkGetSwapchainImagesKHR(device,swapchain,&imgCount,nullptr); std::vector<VkImage> images(imgCount); vkGetSwapchainImagesKHR(device,swapchain,&imgCount,images.data());
    std::vector<VkImageView> views(imgCount);
    for(uint32_t i=0;i<imgCount;i++){ VkImageViewCreateInfo iv{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO}; iv.image=images[i]; iv.viewType=VK_IMAGE_VIEW_TYPE_2D; iv.format=fmt.format; iv.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT; iv.subresourceRange.levelCount=1; iv.subresourceRange.layerCount=1; vkCreateImageView(device,&iv,nullptr,&views[i]); }

    VkAttachmentDescription color{}; color.format=fmt.format; color.samples=VK_SAMPLE_COUNT_1_BIT; color.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR; color.storeOp=VK_ATTACHMENT_STORE_OP_STORE; color.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED; color.finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference ref{0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription sub{}; sub.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS; sub.colorAttachmentCount=1; sub.pColorAttachments=&ref;
    VkRenderPassCreateInfo rpci{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO}; rpci.attachmentCount=1; rpci.pAttachments=&color; rpci.subpassCount=1; rpci.pSubpasses=&sub;
    VkRenderPass rp; vkCreateRenderPass(device,&rpci,nullptr,&rp);

    std::vector<VkFramebuffer> fbs(imgCount);
    for(uint32_t i=0;i<imgCount;i++){ VkFramebufferCreateInfo fb{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO}; fb.renderPass=rp; fb.attachmentCount=1; fb.pAttachments=&views[i]; fb.width=sci.imageExtent.width; fb.height=sci.imageExtent.height; fb.layers=1; vkCreateFramebuffer(device,&fb,nullptr,&fbs[i]); }

    VkCommandPoolCreateInfo cp{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO}; cp.queueFamilyIndex=graphicsQueue; VkCommandPool pool; vkCreateCommandPool(device,&cp,nullptr,&pool);
    VkCommandBufferAllocateInfo cbai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO}; cbai.commandPool=pool; cbai.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY; cbai.commandBufferCount=imgCount; std::vector<VkCommandBuffer> cmds(imgCount); vkAllocateCommandBuffers(device,&cbai,cmds.data());

    for(uint32_t i=0;i<imgCount;i++){ VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO}; vkBeginCommandBuffer(cmds[i],&bi); VkClearValue clear; clear.color={{0.1f,0.2f,0.3f,1.0f}}; VkRenderPassBeginInfo rbi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO}; rbi.renderPass=rp; rbi.framebuffer=fbs[i]; rbi.renderArea.extent=sci.imageExtent; rbi.clearValueCount=1; rbi.pClearValues=&clear; vkCmdBeginRenderPass(cmds[i],&rbi,VK_SUBPASS_CONTENTS_INLINE); dbg.beginLabel(cmds[i],"DrawChunks"); // placeholder for chunk rendering
        // TODO: bind PBR pipeline and draw chunk meshes here
        dbg.endLabel(cmds[i]); vkCmdEndRenderPass(cmds[i]); vkEndCommandBuffer(cmds[i]); }

#if VOXELVK_HAS_IMGUI
    IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGui_ImplGlfw_InitForVulkan(window,true);
    ImGui_ImplVulkan_InitInfo initInfo{}; initInfo.Instance=instance; initInfo.PhysicalDevice=gpu; initInfo.Device=device; initInfo.Queue=queue; initInfo.MinImageCount=imgCount; initInfo.ImageCount=imgCount; ImGui_ImplVulkan_Init(&initInfo,rp);
#endif

    VkSemaphoreCreateInfo sci2{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO}; VkSemaphore imgAvailable, renderFinished; vkCreateSemaphore(device,&sci2,nullptr,&imgAvailable); vkCreateSemaphore(device,&sci2,nullptr,&renderFinished);

    HotReloadShader shader(device,"shaders/pbr.frag");

    auto lastTime = std::chrono::high_resolution_clock::now();
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        auto now=std::chrono::high_resolution_clock::now();
        float dt=std::chrono::duration<float>(now-lastTime).count(); lastTime=now;
        camera.update(window,dt);
        shader.update();
        uint32_t imgIndex; vkAcquireNextImageKHR(device,swapchain,UINT64_MAX,imgAvailable,VK_NULL_HANDLE,&imgIndex);
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO}; si.waitSemaphoreCount=1; si.pWaitSemaphores=&imgAvailable; si.pWaitDstStageMask=&waitStage; si.commandBufferCount=1; si.pCommandBuffers=&cmds[imgIndex]; si.signalSemaphoreCount=1; si.pSignalSemaphores=&renderFinished; vkQueueSubmit(queue,1,&si,VK_NULL_HANDLE);
        VkPresentInfoKHR pi{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR}; pi.waitSemaphoreCount=1; pi.pWaitSemaphores=&renderFinished; pi.swapchainCount=1; pi.pSwapchains=&swapchain; pi.pImageIndices=&imgIndex; vkQueuePresentKHR(queue,&pi);
        vkQueueWaitIdle(queue);
    }

    vkDeviceWaitIdle(device);
#if VOXELVK_HAS_IMGUI
    ImGui_ImplVulkan_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext();
#endif
    vkDestroySemaphore(device,renderFinished,nullptr); vkDestroySemaphore(device,imgAvailable,nullptr);
    vkDestroyCommandPool(device,pool,nullptr);
    for(auto fb:fbs) vkDestroyFramebuffer(device,fb,nullptr);
    vkDestroyRenderPass(device,rp,nullptr);
    for(auto v:views) vkDestroyImageView(device,v,nullptr);
    vkDestroySwapchainKHR(device,swapchain,nullptr);
    vkDestroyDevice(device,nullptr);
    vkDestroySurfaceKHR(instance,surface,nullptr);
    vkDestroyInstance(instance,nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

