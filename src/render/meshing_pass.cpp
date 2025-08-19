
#include "meshing_pass.hpp"
#include <vector>
#include <cstdio>

namespace voxelvk {

static std::vector<uint32_t> load_spirv_file(const char* path) {
    std::vector<uint32_t> data;
    FILE* f = fopen(path, "rb");
    if (!f) { std::fprintf(stderr, "[meshing_pass] Could not open %s\n", path); return data; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    if (sz < 0 || sz % 4 != 0) {
        std::fprintf(stderr, "[meshing_pass] Invalid file size for %s (%ld bytes)\n", path, sz);
        fclose(f);
        return data;
    }
    data.resize((size_t)sz / 4);
    size_t bytes_read = fread(data.data(), 1, (size_t)sz, f);
    fclose(f);
    if (bytes_read != (size_t)sz) {
        std::fprintf(stderr, "[meshing_pass] Failed to read all bytes from %s\n", path);
        data.clear();
    }
    return data;
}

bool MeshingPass::init(VkDevice device, VkPipelineCache cache) {
    m_device = device;
    VkDescriptorSetLayoutBinding b[4] = {};
    for (int i = 0; i < 4; ++i) {
        b[i].binding = i;
        b[i].descriptorCount = 1;
        b[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        b[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    }
    VkDescriptorSetLayoutCreateInfo dsl{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    dsl.bindingCount = 4; dsl.pBindings = b;
    if (vkCreateDescriptorSetLayout(device, &dsl, nullptr, &m_dset_layout) != VK_SUCCESS) return false;

    VkPipelineLayoutCreateInfo plci{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    plci.setLayoutCount = 1; plci.pSetLayouts = &m_dset_layout;
    if (vkCreatePipelineLayout(device, &plci, nullptr, &m_pipe_layout) != VK_SUCCESS) return false;

    auto spirv = load_spirv_file(spirv_path);
    if (spirv.empty()) return false;
    VkShaderModuleCreateInfo smci{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    smci.codeSize = spirv.size() * 4; smci.pCode = spirv.data();
    VkShaderModule sm;
    if (vkCreateShaderModule(device, &smci, nullptr, &sm) != VK_SUCCESS) return false;
    VkComputePipelineCreateInfo cpci{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
    cpci.stage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    cpci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    cpci.stage.module = sm; cpci.stage.pName = "main";
    cpci.layout = m_pipe_layout;
    VkResult pres = vkCreateComputePipelines(device, cache, 1, &cpci, nullptr, &m_pipeline);
    vkDestroyShaderModule(device, sm, nullptr);
    if (pres != VK_SUCCESS) return false;

    VkDescriptorPoolSize ps{}; ps.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; ps.descriptorCount = 4;
    VkDescriptorPoolCreateInfo dpci{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    dpci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    dpci.maxSets = 1; dpci.poolSizeCount = 1; dpci.pPoolSizes = &ps;
    if (vkCreateDescriptorPool(device, &dpci, nullptr, &m_pool) != VK_SUCCESS) return false;
    return true;
}

void MeshingPass::destroy(VkDevice device) {
    if (m_pool) vkDestroyDescriptorPool(device, m_pool, nullptr);
    if (m_pipeline) vkDestroyPipeline(device, m_pipeline, nullptr);
    if (m_pipe_layout) vkDestroyPipelineLayout(device, m_pipe_layout, nullptr);
    if (m_dset_layout) vkDestroyDescriptorSetLayout(device, m_dset_layout, nullptr);
    m_pool = m_pipeline = m_pipe_layout = m_dset_layout = VK_NULL_HANDLE;
    m_device = VK_NULL_HANDLE;
}

void MeshingPass::dispatch(VkCommandBuffer cmd,
                           VkBuffer voxels,
                           VkBuffer vertexBuffer,
                           VkBuffer indexBuffer,
                           VkBuffer counterBuffer) {
    VkDescriptorSetAllocateInfo dsai{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    dsai.descriptorPool = m_pool; dsai.descriptorSetCount = 1; dsai.pSetLayouts = &m_dset_layout;
    VkDescriptorSet ds;
    if (vkAllocateDescriptorSets(m_device, &dsai, &ds) != VK_SUCCESS) return;

    auto sb = [](VkBuffer buf) { VkDescriptorBufferInfo bi{}; bi.buffer = buf; bi.range = VK_WHOLE_SIZE; return bi; };
    VkDescriptorBufferInfo infos[4] = { sb(voxels), sb(vertexBuffer), sb(indexBuffer), sb(counterBuffer) };
    VkWriteDescriptorSet w[4]{};
    for (int i = 0; i < 4; ++i) {
        w[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; w[i].dstSet = ds;
        w[i].dstBinding = i; w[i].descriptorCount = 1;
        w[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; w[i].pBufferInfo = &infos[i];
    }
    vkUpdateDescriptorSets(m_device, 4, w, 0, nullptr);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipe_layout, 0, 1, &ds, 0, nullptr);
    vkCmdDispatch(cmd, DISPATCH_X, DISPATCH_Y, DISPATCH_Z);
    vkFreeDescriptorSets(m_device, m_pool, 1, &ds);
}

} // namespace voxelvk
