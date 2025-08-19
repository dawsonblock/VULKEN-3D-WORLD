#include <cstdio>
#include <vulkan/vulkan.h>
#include "render/frame_graph.hpp"

int main(){
    voxelvk::FrameGraph fg;
    voxelvk::register_default_passes(fg);
    voxelvk::build_and_execute(fg, VK_NULL_HANDLE);
    std::puts("smoke_headless: OK");
    return 0;
}
