#include "resource_registry.hpp"

namespace voxelvk {

ResourceRegistry& ResourceRegistry::instance() {
    static ResourceRegistry g_instance;
    return g_instance;
}

} // namespace voxelvk

