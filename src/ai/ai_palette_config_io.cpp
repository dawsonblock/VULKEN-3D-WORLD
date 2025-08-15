#include "ai_palette_config_io.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
namespace voxelvk::ai {
static inline std::string trim(std::string s){
  auto issp=[](unsigned char c){return std::isspace(c);};
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](unsigned char c){return !issp(c);}));
  s.erase(std::find_if(s.rbegin(), s.rend(), [&](unsigned char c){return !issp(c);} ).base(), s.end());
  return s;
}
static inline std::string lower(std::string s){ std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){return std::tolower(c);}); return s; }
std::string default_palette_ini(){
  return "[palette]\nAir=0\nWater=1\nSand=2\nGrass=3\nDirt=4\nStone=5\nSnow=6\nWood=7\nLeaves=8\nClay=9\n\n"
         "[thresholds]\nsea_level=72\nshoreline_margin=3\nbeach_depth=2\ndirt_depth=4\nsnow_line=140\ncliff_slope_threshold=0.8\n\n"
         "[biomes]\nPlains=Grass\nDesert=Sand\nTaiga=Grass\nSnow=Snow\nSwamp=Grass\nMountain=Stone\nCustom0=Grass\nCustom1=Grass\n";
}
static uint16_t name_to_block_id(const std::string& name, const BlockPalette& p){
  std::string n=lower(name);
  if(n=="air")return p.Air; if(n=="water")return p.Water; if(n=="sand")return p.Sand; if(n=="grass")return p.Grass; if(n=="dirt")return p.Dirt;
  if(n=="stone")return p.Stone; if(n=="snow")return p.Snow; if(n=="wood")return p.Wood; if(n=="leaves")return p.Leaves; if(n=="clay")return p.Clay;
  try{ return (uint16_t)std::stoi(n);}catch(...){ return p.Grass; }
}
bool parse_ini_to_palette(const std::string& text, PaletteConfig& out){
  BlockPalette pal=out.ids; auto &map=out.biome_surface_map;
  enum Sec{NONE,PAL,THR,BIO}; Sec sec=NONE; std::istringstream iss(text); std::string line;
  while(std::getline(iss,line)){
    line=trim(line); if(line.empty()||line[0]=='#'||line[0]==';') continue;
    if(line.front()=='['&&line.back()==']'){ auto s=lower(line.substr(1,line.size()-2)); sec=(s=="palette")?PAL:(s=="thresholds")?THR:(s=="biomes")?BIO:NONE; continue; }
    auto eq=line.find('='); if(eq==std::string::npos) continue;
    std::string k=trim(line.substr(0,eq)); std::string v=trim(line.substr(eq+1));
    if(sec==PAL){
      try{ int id=std::stoi(v);
        if(k=="Air") pal.Air=id; else if(k=="Water") pal.Water=id; else if(k=="Sand") pal.Sand=id; else if(k=="Grass") pal.Grass=id; else if(k=="Dirt") pal.Dirt=id;
        else if(k=="Stone") pal.Stone=id; else if(k=="Snow") pal.Snow=id; else if(k=="Wood") pal.Wood=id; else if(k=="Leaves") pal.Leaves=id; else if(k=="Clay") pal.Clay=id;
      }catch(...){
        uint16_t id=name_to_block_id(v,pal);
        if(k=="Air") pal.Air=id; else if(k=="Water") pal.Water=id; else if(k=="Sand") pal.Sand=id; else if(k=="Grass") pal.Grass=id; else if(k=="Dirt") pal.Dirt=id;
        else if(k=="Stone") pal.Stone=id; else if(k=="Snow") pal.Snow=id; else if(k=="Wood") pal.Wood=id; else if(k=="Leaves") pal.Leaves=id; else if(k=="Clay") pal.Clay=id;
      }
    } else if(sec==THR){
      if(k=="sea_level") out.sea_level=std::stoi(v);
      else if(k=="shoreline_margin") out.shoreline_margin=std::stoi(v);
      else if(k=="beach_depth") out.beach_depth=std::stoi(v);
      else if(k=="dirt_depth") out.dirt_depth=std::stoi(v);
      else if(k=="snow_line") out.snow_line=std::stoi(v);
      else if(k=="cliff_slope_threshold") out.cliff_slope_threshold=std::stof(v);
    } else if(sec==BIO){
      uint16_t id=name_to_block_id(v,pal);
      if(k=="Plains") map[0]=id; else if(k=="Desert") map[1]=id; else if(k=="Taiga") map[2]=id; else if(k=="Snow") map[3]=id;
      else if(k=="Swamp") map[4]=id; else if(k=="Mountain") map[5]=id; else if(k=="Custom0") map[6]=id; else if(k=="Custom1") map[7]=id;
    }
  }
  out.ids=pal; return true;
}
std::string to_string(const PaletteConfig& c){
  std::ostringstream oss;
  oss << "[palette]\n"
      << "Air="<<c.ids.Air<<"\nWater="<<c.ids.Water<<"\nSand="<<c.ids.Sand<<"\nGrass="<<c.ids.Grass<<"\nDirt="<<c.ids.Dirt<<"\nStone="<<c.ids.Stone
      << "\nSnow="<<c.ids.Snow<<"\nWood="<<c.ids.Wood<<"\nLeaves="<<c.ids.Leaves<<"\nClay="<<c.ids.Clay<<"\n\n"
      << "[thresholds]\n"
      << "sea_level="<<c.sea_level<<"\nshoreline_margin="<<c.shoreline_margin<<"\nbeach_depth="<<c.beach_depth
      << "\ndirt_depth="<<c.dirt_depth<<"\nsnow_line="<<c.snow_line<<"\ncliff_slope_threshold="<<c.cliff_slope_threshold<<"\n\n"
      << "[biomes]\n"
      << "Plains="<<c.biome_surface_map[0]<<"\nDesert="<<c.biome_surface_map[1]<<"\nTaiga="<<c.biome_surface_map[2]
      << "\nSnow="<<c.biome_surface_map[3]<<"\nSwamp="<<c.biome_surface_map[4]<<"\nMountain="<<c.biome_surface_map[5]
      << "\nCustom0="<<c.biome_surface_map[6]<<"\nCustom1="<<c.biome_surface_map[7]<<"\n";
  return oss.str();
}
bool PaletteRuntime::load_from_file(const std::string& p){
  std::ifstream ifs(p);
  if(!ifs){
    std::ofstream ofs(p); ofs<<default_palette_ini(); ofs.close();
    ifs.open(p); if(!ifs) return false;
  }
  std::string text((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
  PaletteConfig tmp=cfg; if(!parse_ini_to_palette(text,tmp)) return False;
  cfg=tmp; path=p;
  std::error_code ec; last_write = std::filesystem::last_write_time(p, ec);
  return true;
}
bool PaletteRuntime::save_to_file(const std::string& p) const { std::ofstream ofs(p); if(!ofs) return false; ofs<<to_string(cfg); return true; }
bool PaletteRuntime::tick_hot_reload(){
  std::error_code ec; auto now=std::filesystem::last_write_time(path, ec);
  if(ec) return false; if(last_write==std::filesystem::file_time_type{}){ last_write=now; return false; }
  if(now!=last_write){ last_write=now; return load_from_file(path); } return false;
}
} // ns
