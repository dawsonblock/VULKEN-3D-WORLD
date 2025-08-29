#include "src/vk/vma_helpers.hpp"
#include <iostream>

int main() {
    std::cout << "VMA integration test - compilation successful!" << std::endl;
    std::cout << "Official VMA header is being used." << std::endl;
    
    // Test that we can create the structure types from official VMA
    VmaAllocatorCreateInfo info{};
    voxelvk::VmaBundle bundle{};
    
    std::cout << "VMA structures available and accessible!" << std::endl;
    return 0;
}