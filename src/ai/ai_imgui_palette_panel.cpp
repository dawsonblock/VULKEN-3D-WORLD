#include "ai_imgui_palette_panel.hpp"
#if defined(ENABLE_IMGUI_OVERLAY) && ENABLE_IMGUI_OVERLAY
# if __has_include("imgui.h")
#  include "imgui.h"
#  define VOXELVK_HAS_IMGUI 1
# endif
#endif
namespace voxelvk::ai {
#if VOXELVK_HAS_IMGUI
static bool InputU16(const char* label, uint16_t* v){ int tmp=(int)*v; bool changed=ImGui::InputInt(label,&tmp); if(tmp<0)tmp=0; if(tmp>65535)tmp=65535; *v=(uint16_t)tmp; return changed; }
bool DrawAIPalettePanel(PaletteRuntime& rt){
  if(!ImGui::Begin("AI Palette/World Settings")){ ImGui::End(); return false; }
  bool dirty=false; auto& ids=rt.cfg.ids;
  dirty|=InputU16("Air",&ids.Air); dirty|=InputU16("Water",&ids.Water); dirty|=InputU16("Sand",&ids.Sand);
  dirty|=InputU16("Grass",&ids.Grass); dirty|=InputU16("Dirt",&ids.Dirt); dirty|=InputU16("Stone",&ids.Stone);
  dirty|=InputU16("Snow",&ids.Snow); dirty|=InputU16("Wood",&ids.Wood); dirty|=InputU16("Leaves",&ids.Leaves); dirty|=InputU16("Clay",&ids.Clay);
  ImGui::End(); return dirty;
}
#else
bool DrawAIPalettePanel(PaletteRuntime&){ return false; }
#endif
} // ns
