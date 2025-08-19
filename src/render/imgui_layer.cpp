#include "render/imgui_layer.hpp"

#if defined(ENABLE_IMGUI_OVERLAY) && ENABLE_IMGUI_OVERLAY
# if __has_include("imgui.h")
#  include "imgui.h"
#  define VOXELVK_HAS_IMGUI 1
# endif
#endif
#ifndef VOXELVK_HAS_IMGUI
#define VOXELVK_HAS_IMGUI 0
#endif

namespace voxelvk::render {

void ImGuiLayer::begin() {
#if VOXELVK_HAS_IMGUI
  ImGui::NewFrame();
#endif
}

void ImGuiLayer::end() {
#if VOXELVK_HAS_IMGUI
  ImGui::Render();
#endif
}

void ImGuiLayer::draw(bool &reloadRequest, bool &autoReload, bool &debugToggle) {
#if VOXELVK_HAS_IMGUI
  if (ImGui::Begin("Debug")) {
    if (ImGui::Button("Reload Shaders")) reloadRequest = true;
    ImGui::Checkbox("Auto Reload", &autoReload);
    ImGui::Checkbox("Debug", &debugToggle);
    ImGui::End();
  }
#else
  (void)reloadRequest; (void)autoReload; (void)debugToggle;
#endif
}

} // namespace voxelvk::render
