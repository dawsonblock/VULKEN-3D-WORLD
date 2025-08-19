#pragma once
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

namespace voxelvk {

struct FrameGraphPass {
    std::string name;
    std::vector<std::string> reads;
    std::vector<std::string> writes;
    std::function<void()> execute;
};

// Simple frame graph orchestrating registered passes.
class FrameGraph {
public:
    void addPass(const FrameGraphPass& pass);
    void compile();
    void run();

private:
    std::vector<FrameGraphPass> order;
};

} // namespace voxelvk

