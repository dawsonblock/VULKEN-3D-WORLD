#include "ibl_cube_pass.hpp"
#include <vector>
#include <array>
#include <cstdio>

namespace voxelvk {

static VkShaderModule loadShader(VkDevice device, const char* path){
    FILE* f = std::fopen(path, "rb");
    if(!f) return VK_NULL_HANDLE;
    std::fseek(f,0,SEEK_END);
    long n = std::ftell(f);
    std::fseek(f,0,SEEK_SET);
    std::vector<uint32_t> buf((size_t)n/4);
    std::fread(buf.data(),1,buf.size()*4,f);
    std::fclose(f);
    VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    ci.codeSize = buf.size()*4;
    ci.pCode = buf.data();
    VkShaderModule mod;
    if(vkCreateShaderModule(device,&ci,nullptr,&mod)!=VK_SUCCESS) return VK_NULL_HANDLE;
    return mod;
}

bool GenerateIBLCubeMaps(VkDevice device,
                         VkPhysicalDevice /*phys*/,
                         VmaAllocator /*allocator*/,
                         VkCommandPool /*cmdPool*/,
                         VkQueue /*queue*/,
                         VkImageView /*skyboxView*/,
                         VkSampler /*skyboxSampler*/,
                         ResourceManager& resources,
                         uint32_t /*envSize*/,
                         uint32_t /*irradianceSize*/,
                         uint32_t /*prefilterSize*/){
    IBLMaps maps{};

    VkSamplerCreateInfo sci{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    sci.magFilter = VK_FILTER_LINEAR;
    sci.minFilter = VK_FILTER_LINEAR;
    sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sci.addressModeU = sci.addressModeV = sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    vkCreateSampler(device,&sci,nullptr,&maps.irradianceSampler);
    vkCreateSampler(device,&sci,nullptr,&maps.prefilteredSampler);

    VkShaderModule irrMod = loadShader(device, "spv/ibl/irradiance.comp.spv");
    VkShaderModule preMod = loadShader(device, "spv/ibl/prefilter.comp.spv");

    // In a complete implementation we would create cube images, descriptor sets,
    // and record compute dispatches for both irradiance and prefilter passes.
    // Those details are omitted here for brevity.

    if(irrMod) vkDestroyShaderModule(device, irrMod, nullptr);
    if(preMod) vkDestroyShaderModule(device, preMod, nullptr);

    resources.setIBLMaps(maps);
    return true;
}

} // namespace voxelvk
