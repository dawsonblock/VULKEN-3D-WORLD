#!/usr/bin/env python3
"""
VoxelVK Elite ALL - Backend API Server
Provides API endpoints for the graphics engine dashboard.
"""

from fastapi import FastAPI, HTTPException
from fastapi.staticfiles import StaticFiles
from fastapi.responses import FileResponse, HTMLResponse
import subprocess
import os
import json
from pathlib import Path
from typing import Dict, List, Any
import asyncio

app = FastAPI(
    title="VoxelVK Elite ALL API",
    description="Backend API for VoxelVK Elite ALL Graphics Engine Dashboard",
    version="1.0.0"
)

# Mount static files
app.mount("/static", StaticFiles(directory="../frontend"), name="static")

class EngineInfo:
    """Engine information and status manager."""
    
    def __init__(self):
        self.project_root = Path("/app")
        self.build_dir = self.project_root / "build"
        self.shader_dir = self.project_root / "shaders_vk"
        self.spv_dir = self.build_dir / "spv"
        
    def get_engine_status(self) -> Dict[str, Any]:
        """Get overall engine status and information."""
        return {
            "status": "online" if self.build_dir.exists() else "offline",
            "version": "3.0.0",
            "build_type": "RelWithDebInfo",
            "vulkan_version": "1.3.239.0",
            "vma_status": "Official VMA" if self._check_vma_status() else "VMA Shim",
            "build_time": self._get_build_time()
        }
    
    def _check_vma_status(self) -> bool:
        """Check if official VMA is being used."""
        external_vma = self.project_root / "external" / "vma" / "vk_mem_alloc.h"
        if external_vma.exists():
            # Check file size to distinguish between official and shim
            return external_vma.stat().st_size > 100000  # Official VMA is ~750KB
        return False
    
    def _get_build_time(self) -> str:
        """Get the last build time."""
        if self.build_dir.exists():
            build_files = list(self.build_dir.glob("*"))
            if build_files:
                latest = max(build_files, key=lambda x: x.stat().st_mtime)
                from datetime import datetime
                return datetime.fromtimestamp(latest.stat().st_mtime).isoformat()
        return "Unknown"
    
    def get_shader_status(self) -> List[Dict[str, str]]:
        """Get shader compilation status."""
        shaders = []
        
        if self.shader_dir.exists():
            # Find all GLSL files
            for shader_file in self.shader_dir.rglob("*.glsl"):
                relative_path = shader_file.relative_to(self.shader_dir)
                
                # Check if corresponding SPIR-V exists
                spv_path = self.spv_dir / f"{relative_path.stem}.spv"
                status = "success" if spv_path.exists() else "error"
                
                shaders.append({
                    "name": str(relative_path),
                    "status": status,
                    "path": str(shader_file),
                    "spv_path": str(spv_path) if spv_path.exists() else None
                })
        
        return sorted(shaders, key=lambda x: x["name"])
    
    def get_system_info(self) -> Dict[str, Any]:
        """Get system and Vulkan information."""
        info = {
            "gpu_name": "Unknown",
            "driver_version": "Unknown", 
            "gpu_memory": "Unknown",
            "vulkan_extensions": "Unknown"
        }
        
        try:
            # Try to get Vulkan info if available
            result = subprocess.run(
                ["vulkaninfo", "--summary"],
                capture_output=True,
                text=True,
                timeout=5
            )
            if result.returncode == 0:
                # Parse vulkaninfo output for basic info
                lines = result.stdout.split('\n')
                for line in lines:
                    if "deviceName" in line:
                        info["gpu_name"] = line.split('=')[-1].strip()
                    elif "driverVersion" in line:
                        info["driver_version"] = line.split('=')[-1].strip()
        except (subprocess.TimeoutExpired, subprocess.SubprocessError, FileNotFoundError):
            # Vulkaninfo not available or timed out
            info["gpu_name"] = "Vulkan Device (Headless)"
            info["driver_version"] = "1.3.239.0"
            info["gpu_memory"] = "Simulated"
            info["vulkan_extensions"] = "Standard Extensions"
        
        return info
    
    def get_build_info(self) -> Dict[str, Any]:
        """Get CMake build information."""
        cmake_cache = self.build_dir / "CMakeCache.txt"
        build_info = {
            "cmake_version": "Unknown",
            "compiler": "Unknown",
            "build_flags": [],
            "targets_built": []
        }
        
        if cmake_cache.exists():
            try:
                with open(cmake_cache) as f:
                    content = f.read()
                    
                # Extract key information
                if "CMAKE_VERSION:INTERNAL=" in content:
                    for line in content.split('\n'):
                        if line.startswith("CMAKE_VERSION:INTERNAL="):
                            build_info["cmake_version"] = line.split('=')[1]
                        elif line.startswith("CMAKE_CXX_COMPILER:FILEPATH="):
                            compiler_path = line.split('=')[1]
                            build_info["compiler"] = Path(compiler_path).name
                            
            except Exception as e:
                print(f"Error reading CMakeCache.txt: {e}")
        
        # Check for built targets
        if self.build_dir.exists():
            executables = list(self.build_dir.glob("*"))
            build_info["targets_built"] = [
                f.name for f in executables 
                if f.is_file() and (f.suffix == '' or f.suffix == '.exe')
            ]
        
        return build_info

# Global engine info instance
engine = EngineInfo()

@app.get("/", response_class=HTMLResponse)
async def serve_dashboard():
    """Serve the main dashboard."""
    dashboard_path = Path("frontend/index.html")
    if dashboard_path.exists():
        return FileResponse(dashboard_path)
    else:
        return HTMLResponse("<h1>VoxelVK Dashboard Not Found</h1><p>Please ensure frontend files are available.</p>")

@app.get("/api/engine/status")
async def get_engine_status():
    """Get engine status and basic information."""
    return engine.get_engine_status()

@app.get("/api/engine/shaders")
async def get_shader_status():
    """Get shader compilation status."""
    return {
        "shaders": engine.get_shader_status(),
        "total_shaders": len(engine.get_shader_status()),
        "compiled_shaders": len([s for s in engine.get_shader_status() if s["status"] == "success"])
    }

@app.get("/api/engine/system")
async def get_system_info():
    """Get system and hardware information."""
    return engine.get_system_info()

@app.get("/api/engine/build")
async def get_build_info():
    """Get build system information."""
    return engine.get_build_info()

@app.get("/api/engine/performance")
async def get_performance_metrics():
    """Get simulated performance metrics."""
    # In a real implementation, this would interface with the C++ engine
    import random
    import time
    
    if engine.get_engine_status()["status"] == "online":
        return {
            "frame_time_ms": round(random.uniform(12.0, 20.0), 1),
            "fps": random.randint(50, 80),
            "draw_calls": random.randint(200, 700),
            "triangles": random.randint(10000, 60000),
            "memory_usage": {
                "total_mb": random.randint(512, 2048),
                "device_mb": random.randint(300, 1400),
                "host_mb": random.randint(100, 600)
            },
            "timestamp": time.time()
        }
    else:
        return {
            "frame_time_ms": 0.0,
            "fps": 0,
            "draw_calls": 0,
            "triangles": 0,
            "memory_usage": {"total_mb": 0, "device_mb": 0, "host_mb": 0},
            "timestamp": time.time()
        }

@app.post("/api/engine/config")
async def apply_engine_config(config: Dict[str, Any]):
    """Apply engine configuration settings."""
    # In a real implementation, this would interface with the C++ engine
    # For now, just validate and return success
    
    valid_keys = {"lod_level", "shadow_quality", "enable_validation", "memory_budget"}
    if not all(key in valid_keys for key in config.keys()):
        raise HTTPException(status_code=400, detail="Invalid configuration keys")
    
    return {
        "success": True,
        "message": "Configuration applied successfully",
        "applied_config": config
    }

@app.get("/api/engine/logs")
async def get_engine_logs():
    """Get recent engine logs."""
    # In a real implementation, this would read from engine log files
    return {
        "logs": [
            {"level": "INFO", "message": "VMA allocator initialized", "timestamp": "2025-08-29T02:00:00Z"},
            {"level": "INFO", "message": "Vulkan device created successfully", "timestamp": "2025-08-29T02:00:01Z"},
            {"level": "INFO", "message": "Shaders compiled successfully", "timestamp": "2025-08-29T02:00:02Z"},
            {"level": "INFO", "message": "Engine ready", "timestamp": "2025-08-29T02:00:03Z"}
        ]
    }

@app.get("/health")
async def health_check():
    """Health check endpoint."""
    return {"status": "healthy", "service": "VoxelVK Elite ALL API"}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8001)