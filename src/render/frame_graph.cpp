#include "frame_graph.hpp"
#include <algorithm>

namespace voxelvk {

void FrameGraph::addPass(const FrameGraphPass& pass){
    order.push_back(pass);
}

void FrameGraph::compile(){
    // Minimal dependency resolution: stable order with writes before reads.
    std::stable_sort(order.begin(), order.end(), [](const FrameGraphPass& a, const FrameGraphPass& b){
        for(const auto& w : a.writes){
            if(std::find(b.reads.begin(), b.reads.end(), w) != b.reads.end())
                return true;
        }
        return false;
    });
}

void FrameGraph::run(){
    for(auto& p : order){
        if(p.execute) p.execute();
    }
}

} // namespace voxelvk

