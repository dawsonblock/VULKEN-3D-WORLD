#include "lod_component.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace voxelvk {

LODComponent::LODComponent(const std::string& cfgPath) {
    LoadConfig(cfgPath);
}

void LODComponent::LoadConfig(const std::string& cfgPath) {
    std::ifstream f(cfgPath);
    if(!f.is_open()) {
        throw std::runtime_error("Could not open config file '" + cfgPath + "' in LODComponent::LoadConfig");
    }
    std::string line;
    while(std::getline(f, line)){
        if(line.empty() || line[0]=='#') continue;
        if(line[0]=='[') continue;
        std::istringstream iss(line);
        std::string key; char eq; float val;
        if(iss >> key >> eq >> val && eq=='='){
            if(key == "high_detail") cfg_.high_detail = val;
            else if(key == "medium_detail") cfg_.medium_detail = val;
        }
    }
}

LODLevel LODComponent::Select(float distance) const {
    if(distance < cfg_.high_detail) return LODLevel::High;
    if(distance < cfg_.medium_detail) return LODLevel::Medium;
    return LODLevel::Low;
}

} // namespace voxelvk
