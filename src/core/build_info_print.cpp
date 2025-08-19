
#include "build_info_print.hpp"
#include "build_info.hpp"
#include <mutex>
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace voxelvk {
static std::once_flag g_once;
static std::shared_ptr<spdlog::logger> g_logger;

void InitBuildInfoLogging(){
    if(!g_logger){
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("debug_vk_runtime.log", true);
        g_logger = std::make_shared<spdlog::logger>("build", spdlog::sinks_init_list{console_sink, file_sink});
        g_logger->set_pattern("%v");
        g_logger->flush_on(spdlog::level::info);
        spdlog::set_default_logger(g_logger);
    }
}

void ShutdownBuildInfoLogging(){ g_logger.reset(); }

void LogLine(const char* line){
    if(!g_logger) InitBuildInfoLogging();
    g_logger->info(line);
}

static void _print(){
    spdlog::info("[BUILD INFO]");
    spdlog::info("Branch Sources: {}", VOXELVK_BUILD_BRANCH_SOURCES);
    spdlog::info("Build Time: {}", VOXELVK_BUILD_TIME);
    spdlog::info("Build Hash: {}", VOXELVK_BUILD_HASH);
    spdlog::info("Vulkan SDK: {}", VOXELVK_VULKAN_SDK_VERSION);
#if VOXELVK_GPU_COMPAT_LAYER
    spdlog::info("GPU Compatibility Layer: ENABLED");
#else
    spdlog::info("GPU Compatibility Layer: DISABLED");
#endif
#if VOXELVK_RUNTIME_DEBUG_TOGGLE
    spdlog::info("Runtime Vulkan Debug Toggle: ENABLED (Hotkey: F9)");
#else
    spdlog::info("Runtime Vulkan Debug Toggle: DISABLED");
#endif
}

void PrintBuildInfoOnce(){ std::call_once(g_once, [](){ InitBuildInfoLogging(); _print(); }); }
void PrintBuildInfoAlways(){ InitBuildInfoLogging(); _print(); }

} // namespace voxelvk
