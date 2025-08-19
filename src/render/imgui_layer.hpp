#pragma once

#include <stdbool.h>

namespace voxelvk::render {
struct ImGuiLayer {
  void begin();
  void end();
  void draw(bool &reloadRequest, bool &autoReload, bool &debugToggle);
};
} // namespace voxelvk::render

