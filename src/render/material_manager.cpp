#include "material_manager.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>

#include "vk_mem_alloc.h"

namespace voxelvk {

namespace {
struct ParsedMaterial {
    glm::vec3 albedo{0.0f};
    float metallic{0.0f};
    float roughness{1.0f};
};

// Very small JSON parser tailored for materials.json
static std::unordered_map<std::string, ParsedMaterial>
parse_materials_file(const std::string& path){
    std::unordered_map<std::string, ParsedMaterial> out;
    std::ifstream ifs(path);
    if(!ifs) return out;
    std::string contents((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    size_t pos = contents.find("\"materials\"");
    if(pos == std::string::npos) return out;
    pos = contents.find('{', pos);
    if(pos == std::string::npos) return out;
    ++pos;
    while(true){
        pos = contents.find('"', pos);
        if(pos == std::string::npos) break;
        size_t endName = contents.find('"', pos+1);
        if(endName == std::string::npos) break;
        std::string name = contents.substr(pos+1, endName-pos-1);
        pos = contents.find('{', endName);
        if(pos == std::string::npos) break;
        size_t blockEnd = contents.find('}', pos);
        if(blockEnd == std::string::npos) break;
        std::string block = contents.substr(pos+1, blockEnd-pos-1);

        ParsedMaterial mat;
        size_t aStart = block.find('[');
        size_t aEnd = block.find(']', aStart);
        if(aStart != std::string::npos && aEnd != std::string::npos){
            std::string arr = block.substr(aStart+1, aEnd-aStart-1);
            std::stringstream ss(arr);
            std::string item;
            for(int i=0;i<3 && std::getline(ss,item,',');++i){
                mat.albedo[i] = std::stof(item);
            }
        }
        size_t mPos = block.find("\"metallic\"");
        if(mPos != std::string::npos){
            mPos = block.find(':', mPos);
            size_t mEnd = block.find(',', mPos);
            if(mEnd==std::string::npos) mEnd = block.size();
            mat.metallic = std::stof(block.substr(mPos+1, mEnd-mPos-1));
        }
        size_t rPos = block.find("\"roughness\"");
        if(rPos != std::string::npos){
            rPos = block.find(':', rPos);
            size_t rEnd = block.find(',', rPos);
            if(rEnd==std::string::npos) rEnd = block.size();
            mat.roughness = std::stof(block.substr(rPos+1, rEnd-rPos-1));
        }
        out[name] = mat;
        pos = blockEnd+1;
    }
    return out;
}
} // namespace

bool MaterialManager::init(VkDevice dev, VmaAllocator alloc, const std::string& jsonPath){
    device_ = dev;
    allocator_ = alloc;
    auto parsed = parse_materials_file(jsonPath);
    if(parsed.empty()) return false;

    // descriptor layout
    VkDescriptorSetLayoutBinding b{};
    b.binding = 0;
    b.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    b.descriptorCount = 1;
    b.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo lci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    lci.bindingCount = 1;
    lci.pBindings = &b;
    if(vkCreateDescriptorSetLayout(device_, &lci, nullptr, &setLayout_) != VK_SUCCESS)
        return false;

    // descriptor pool
    VkDescriptorPoolSize ps{};
    ps.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ps.descriptorCount = static_cast<uint32_t>(parsed.size());
    VkDescriptorPoolCreateInfo pci{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pci.poolSizeCount = 1;
    pci.pPoolSizes = &ps;
    pci.maxSets = static_cast<uint32_t>(parsed.size());
    if(vkCreateDescriptorPool(device_, &pci, nullptr, &descriptorPool_) != VK_SUCCESS)
        return false;

    materials_.clear();
    for(const auto& [name, mat] : parsed){
        MaterialRecord rec;
        rec.albedo = mat.albedo;
        rec.metallic = mat.metallic;
        rec.roughness = mat.roughness;

        MaterialGPU gpu{};
        gpu.albedo = glm::vec4(mat.albedo, mat.metallic);
        gpu.params = glm::vec4(mat.roughness,0,0,0);

        VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bi.size = sizeof(MaterialGPU);
        bi.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        VmaAllocationCreateInfo aci{};
        aci.usage = VMA_MEMORY_USAGE_AUTO;
        aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                    VMA_ALLOCATION_CREATE_MAPPED_BIT;
        VmaAllocationInfo info{};
        if(vmaCreateBuffer(allocator_, &bi, &aci, &rec.ubo, &rec.alloc, &info) != VK_SUCCESS)
            return false;
        std::memcpy(info.pMappedData, &gpu, sizeof(gpu));

        VkDescriptorSetAllocateInfo ai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        ai.descriptorPool = descriptorPool_;
        ai.descriptorSetCount = 1;
        ai.pSetLayouts = &setLayout_;
        if(vkAllocateDescriptorSets(device_, &ai, &rec.descSet) != VK_SUCCESS)
            return false;
        VkDescriptorBufferInfo buf{};
        buf.buffer = rec.ubo;
        buf.offset = 0;
        buf.range = sizeof(MaterialGPU);
        VkWriteDescriptorSet w{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        w.dstSet = rec.descSet;
        w.dstBinding = 0;
        w.descriptorCount = 1;
        w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        w.pBufferInfo = &buf;
        vkUpdateDescriptorSets(device_, 1, &w, 0, nullptr);

        materials_.emplace(name, rec);
    }
    return true;
}

void MaterialManager::destroy(){
    for(auto& [name, m] : materials_){
        if(m.ubo) vmaDestroyBuffer(allocator_, m.ubo, m.alloc);
    }
    materials_.clear();
    if(descriptorPool_) vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);
    descriptorPool_ = VK_NULL_HANDLE;
    if(setLayout_) vkDestroyDescriptorSetLayout(device_, setLayout_, nullptr);
    setLayout_ = VK_NULL_HANDLE;
    device_ = VK_NULL_HANDLE;
    allocator_ = nullptr;
}

const MaterialRecord* MaterialManager::get(const std::string& name) const{
    auto it = materials_.find(name);
    if(it==materials_.end()) return nullptr;
    return &it->second;
}

void MaterialManager::bind(VkCommandBuffer cmd, VkPipelineLayout layout, uint32_t setIndex, const std::string& name) const{
    auto m = get(name);
    if(!m) return;
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, setIndex, 1, &m->descSet, 0, nullptr);
}

} // namespace voxelvk

