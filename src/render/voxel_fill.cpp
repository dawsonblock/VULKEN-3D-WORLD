#include "voxel_fill.hpp"
#include "shader_watcher.hpp"

#include <vector>
#include <cstdio>
#include <cstdlib>

namespace voxelvk {

namespace {

static std::vector<uint32_t> load_spirv_file(const char* path) {
    std::vector<uint32_t> data;
    FILE* f = fopen(path, "rb");
    if (!f) { std::fprintf(stderr, "[voxel_fill] Could not open %s\n", path); return data; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    if (sz < 0 || sz % 4 != 0) {
        std::fprintf(stderr, "[voxel_fill] File size of %s is not divisible by 4 or is negative (%ld bytes)\n", path, sz);
        fclose(f);
        return data;
    }
    data.resize(static_cast<size_t>(sz) / 4);
    size_t bytes_read = fread(data.data(), 1, static_cast<size_t>(sz), f);
    fclose(f);
    if (bytes_read != static_cast<size_t>(sz)) {
        std::fprintf(stderr, "[voxel_fill] Failed to read all bytes from %s (read %zu of %ld)\n", path, bytes_read, sz);
        data.clear();
    }
    return data;
}

static bool compile_glsl_to_spirv(const char* src, const char* dst) {
    std::string cmd = "glslc " + std::string(src) + " -o " + dst;
    int res = std::system(cmd.c_str());
    if (res != 0) {
        std::fprintf(stderr, "[voxel_fill] Failed to compile %s\n", src);
        return false;
    }
    return true;
}

} // namespace

bool VoxelFill::create_pipeline() {
    VkDescriptorSetLayoutBinding b[2] = {};
    for (int i = 0; i < 2; ++i) {
        b[i].binding = i; b[i].descriptorCount = 1;
        b[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        b[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    }
    VkDescriptorSetLayoutCreateInfo dsl{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    dsl.bindingCount = 2; dsl.pBindings = b;
    VkResult res = vkCreateDescriptorSetLayout(m_device, &dsl, nullptr, &m_dset_layout);
    if (res != VK_SUCCESS) return false;

    VkPipelineLayoutCreateInfo plci{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    plci.setLayoutCount = 1; plci.pSetLayouts = &m_dset_layout;
    VkResult pl_result = vkCreatePipelineLayout(m_device, &plci, nullptr, &m_pipe_layout);
    if (pl_result != VK_SUCCESS) return false;

    auto spirv = load_spirv_file("spv/voxel_fill.comp.spv");
    if (spirv.empty()) return false;

    VkShaderModuleCreateInfo smci{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    smci.codeSize = spirv.size() * sizeof(uint32_t);
    smci.pCode = spirv.data();
    VkShaderModule sm;
    VkResult sm_res = vkCreateShaderModule(m_device, &smci, nullptr, &sm);
    if (sm_res != VK_SUCCESS) return false;

    VkComputePipelineCreateInfo cpci{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
    cpci.stage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    cpci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    cpci.stage.module = sm; cpci.stage.pName = "main";
    cpci.layout = m_pipe_layout;
    VkResult pipelineResult = vkCreateComputePipelines(m_device, m_cache, 1, &cpci, nullptr, &m_pipeline);
    vkDestroyShaderModule(m_device, sm, nullptr);
    if (pipelineResult != VK_SUCCESS) {
        return false;
    }

    VkDescriptorPoolSize ps{}; ps.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; ps.descriptorCount = 2;
    VkDescriptorPoolCreateInfo dpci{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    dpci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    dpci.maxSets = 1; dpci.poolSizeCount = 1; dpci.pPoolSizes = &ps;
    VkResult poolResult = vkCreateDescriptorPool(m_device, &dpci, nullptr, &m_pool);
    if (poolResult != VK_SUCCESS) return false;
    return true;
}

void VoxelFill::destroy_pipeline() {
    if (m_pool) vkDestroyDescriptorPool(m_device, m_pool, nullptr);
    if (m_pipeline) vkDestroyPipeline(m_device, m_pipeline, nullptr);
    if (m_pipe_layout) vkDestroyPipelineLayout(m_device, m_pipe_layout, nullptr);
    if (m_dset_layout) vkDestroyDescriptorSetLayout(m_device, m_dset_layout, nullptr);
    m_pool = m_pipeline = m_pipe_layout = m_dset_layout = VK_NULL_HANDLE;
}

bool VoxelFill::init(VkDevice device, VkPipelineCache cache) {
    m_device = device;
    m_cache = cache;
    if (!create_pipeline()) return false;

    m_watcher = std::make_unique<ShaderWatcher>(
        std::vector<std::string>{"shaders", "shaders_vk"},
        [this](const std::string& path) {
            if (path.find("voxel_fill.comp.glsl") != std::string::npos) {
                compile_glsl_to_spirv(path.c_str(), "spv/voxel_fill.comp.spv");
                this->reload();
            }
        });
    m_watcher->start();
    return true;
}

bool VoxelFill::reload() {
    destroy_pipeline();
    return create_pipeline();
}

void VoxelFill::destroy(VkDevice device) {
    if (m_watcher) {
        m_watcher->stop();
        m_watcher.reset();
    }
    destroy_pipeline();
    m_device = VK_NULL_HANDLE;
    m_cache = VK_NULL_HANDLE;
}

void VoxelFill::dispatch(VkCommandBuffer cmd,
                         VkImageView surface_r8ui,
                         VkImageView out_r8ui,
                         uint32_t SX, uint32_t SY) {
    VkDescriptorSetAllocateInfo dsai{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    dsai.descriptorPool = m_pool; dsai.descriptorSetCount = 1; dsai.pSetLayouts = &m_dset_layout;
    VkDescriptorSet ds;
    VkResult alloc_result = vkAllocateDescriptorSets(m_device, &dsai, &ds);
    if (alloc_result != VK_SUCCESS) {
        std::fprintf(stderr, "[voxel_fill] vkAllocateDescriptorSets failed with error code %d\n", alloc_result);
        return;
    }

    auto storage_image = [](VkImageView v) {
        VkDescriptorImageInfo ii{}; ii.imageView = v; ii.imageLayout = VK_IMAGE_LAYOUT_GENERAL; return ii; };
    VkDescriptorImageInfo i0 = storage_image(surface_r8ui);
    VkDescriptorImageInfo i1 = storage_image(out_r8ui);
    VkWriteDescriptorSet w[2]{};
    for (int i = 0; i < 2; ++i) {
        w[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; w[i].dstSet = ds;
        w[i].dstBinding = i; w[i].descriptorCount = 1;
        w[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }
    w[0].pImageInfo = &i0; w[1].pImageInfo = &i1; vkUpdateDescriptorSets(m_device, 2, w, 0, nullptr);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipe_layout, 0, 1, &ds, 0, nullptr);
    uint32_t gx = (SX + 7) / 8, gy = (SY + 7) / 8;
    vkCmdDispatch(cmd, gx, gy, 1);
    vkFreeDescriptorSets(m_device, m_pool, 1, &ds);
}

} // namespace voxelvk

