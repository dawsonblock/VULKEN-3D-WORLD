// VoxelVK Elite ALL Dashboard JavaScript
class VoxelVKDashboard {
    constructor() {
        this.engineStatus = 'initializing';
        this.updateInterval = null;
        this.shaderData = [];
        this.performanceMetrics = {
            frameTime: 0,
            fps: 0,
            drawCalls: 0,
            triangles: 0
        };
        
        this.init();
    }

    init() {
        this.setupEventListeners();
        this.updateEngineStatus();
        this.loadShaderInformation();
        this.loadSystemInformation();
        this.startPerformanceMonitoring();
        
        // Simulate engine initialization
        setTimeout(() => {
            this.engineStatus = 'online';
            this.updateStatusIndicator();
        }, 2000);
    }

    setupEventListeners() {
        // Memory budget slider
        const memoryBudgetSlider = document.getElementById('memoryBudget');
        const memoryBudgetValue = document.getElementById('memoryBudgetValue');
        
        memoryBudgetSlider.addEventListener('input', (e) => {
            memoryBudgetValue.textContent = e.target.value;
        });

        // Apply settings button
        document.getElementById('applySettings').addEventListener('click', () => {
            this.applySettings();
        });

        // Configuration change listeners
        ['lodLevel', 'shadowQuality', 'enableValidation'].forEach(id => {
            const element = document.getElementById(id);
            if (element) {
                element.addEventListener('change', () => {
                    this.updateConfigurationPreview();
                });
            }
        });
    }

    updateEngineStatus() {
        const statusElement = document.getElementById('engineStatus');
        const statusText = document.getElementById('statusText');
        
        statusElement.className = `status-light ${this.engineStatus}`;
        
        const statusMessages = {
            'initializing': 'Initializing Engine...',
            'online': 'Engine Online',
            'offline': 'Engine Offline',
            'error': 'Engine Error'
        };
        
        statusText.textContent = statusMessages[this.engineStatus] || 'Unknown Status';
    }

    updateStatusIndicator() {
        this.updateEngineStatus();
        this.updateMemoryStatistics();
        this.updatePerformanceMetrics();
    }

    loadShaderInformation() {
        // Simulate loading shader compilation status
        const shaderList = [
            { name: 'voxelize.vert.glsl', status: 'success' },
            { name: 'voxelize.frag.glsl', status: 'success' },
            { name: 'voxelize_sat.comp.glsl', status: 'success' },
            { name: 'frustum_cull.comp.glsl', status: 'success' },
            { name: 'diff_voxelize.comp.glsl', status: 'success' },
            { name: 'csm_depth.vert.glsl', status: 'success' },
            { name: 'csm_depth.frag.glsl', status: 'success' },
            { name: 'greedy_meshing.comp.glsl', status: 'success' },
            { name: 'sdf_jump_flood.comp.glsl', status: 'success' },
            { name: 'voxel_fill.comp.glsl', status: 'success' },
            { name: 'brdf_lut.comp.glsl', status: 'success' },
            { name: 'atmosphere.frag.glsl', status: 'success' },
            { name: 'clouds.frag.glsl', status: 'success' },
            { name: 'blockize.comp.glsl', status: 'success' },
            { name: 'fullscreen.vert.glsl', status: 'success' }
        ];

        this.shaderData = shaderList;
        this.renderShaderList();
    }

    renderShaderList() {
        const shaderListElement = document.getElementById('shaderList');
        
        if (this.shaderData.length === 0) {
            shaderListElement.innerHTML = '<div class="shader-item"><span class="shader-name">No shaders found</span><span class="compilation-status error">Error</span></div>';
            return;
        }

        shaderListElement.innerHTML = this.shaderData.map(shader => `
            <div class="shader-item">
                <span class="shader-name">${shader.name}</span>
                <span class="compilation-status ${shader.status}">
                    ${shader.status === 'success' ? 'Compiled' : 
                      shader.status === 'error' ? 'Failed' : 'Pending'}
                </span>
            </div>
        `).join('');
    }

    loadSystemInformation() {
        // Simulate system information loading
        setTimeout(() => {
            document.getElementById('gpuName').textContent = 'Simulated Vulkan Device';
            document.getElementById('driverVersion').textContent = '1.3.239.0';
            document.getElementById('gpuMemory').textContent = '8 GB VRAM';
            document.getElementById('vulkanExtensions').textContent = '156 Extensions Available';
        }, 1500);
    }

    updateMemoryStatistics() {
        // Simulate VMA memory statistics
        const totalMemory = Math.floor(Math.random() * 2048) + 512; // 512-2560 MB
        const deviceMemory = Math.floor(totalMemory * 0.7);
        const hostMemory = totalMemory - deviceMemory;

        document.getElementById('totalMemory').textContent = `${totalMemory} MB`;
        document.getElementById('deviceMemory').textContent = `${deviceMemory} MB`;
        document.getElementById('hostMemory').textContent = `${hostMemory} MB`;

        // Update progress bars
        const progressBars = document.querySelectorAll('.memory-item .progress-fill');
        progressBars[0].style.width = `${Math.min((totalMemory / 4096) * 100, 100)}%`;
        progressBars[1].style.width = `${Math.min((deviceMemory / 2048) * 100, 100)}%`;
        progressBars[2].style.width = `${Math.min((hostMemory / 1024) * 100, 100)}%`;
    }

    updatePerformanceMetrics() {
        if (this.engineStatus === 'online') {
            // Simulate realistic performance metrics
            this.performanceMetrics.frameTime = (Math.random() * 8 + 12).toFixed(1); // 12-20ms
            this.performanceMetrics.fps = Math.floor(1000 / this.performanceMetrics.frameTime);
            this.performanceMetrics.drawCalls = Math.floor(Math.random() * 500 + 200); // 200-700
            this.performanceMetrics.triangles = Math.floor(Math.random() * 50000 + 10000); // 10K-60K
        } else {
            this.performanceMetrics = { frameTime: 0, fps: 0, drawCalls: 0, triangles: 0 };
        }

        document.getElementById('frameTime').textContent = this.performanceMetrics.frameTime;
        document.getElementById('fps').textContent = this.performanceMetrics.fps;
        document.getElementById('drawCalls').textContent = this.performanceMetrics.drawCalls.toLocaleString();
        document.getElementById('triangles').textContent = this.performanceMetrics.triangles.toLocaleString();
    }

    startPerformanceMonitoring() {
        if (this.updateInterval) {
            clearInterval(this.updateInterval);
        }

        this.updateInterval = setInterval(() => {
            this.updateStatusIndicator();
        }, 1000);
    }

    applySettings() {
        const settings = {
            lodLevel: document.getElementById('lodLevel').value,
            shadowQuality: document.getElementById('shadowQuality').value,
            enableValidation: document.getElementById('enableValidation').checked,
            memoryBudget: document.getElementById('memoryBudget').value
        };

        console.log('Applying settings:', settings);
        
        // Show notification
        this.showNotification('Settings applied successfully!', 'success');
        
        // Simulate engine restart with new settings
        this.engineStatus = 'initializing';
        this.updateStatusIndicator();
        
        setTimeout(() => {
            this.engineStatus = 'online';
            this.updateStatusIndicator();
        }, 2000);
    }

    updateConfigurationPreview() {
        const lodLevel = document.getElementById('lodLevel').value;
        const shadowQuality = document.getElementById('shadowQuality').value;
        const enableValidation = document.getElementById('enableValidation').checked;
        
        // Update performance estimation based on settings
        let performanceMultiplier = 1.0;
        performanceMultiplier -= (parseInt(lodLevel) * 0.1); // Lower LOD = better performance
        
        switch(shadowQuality) {
            case 'low': performanceMultiplier += 0.2; break;
            case 'medium': performanceMultiplier += 0.1; break;
            case 'high': performanceMultiplier -= 0.1; break;
            case 'ultra': performanceMultiplier -= 0.3; break;
        }
        
        if (enableValidation) {
            performanceMultiplier -= 0.15; // Validation layers impact performance
        }
        
        // Visual feedback for performance impact
        const applyButton = document.getElementById('applySettings');
        if (performanceMultiplier < 0.7) {
            applyButton.style.background = 'linear-gradient(135deg, #e74c3c, #c0392b)';
            applyButton.textContent = 'Apply Settings (Performance Impact)';
        } else if (performanceMultiplier > 1.2) {
            applyButton.style.background = 'linear-gradient(135deg, #27ae60, #229954)';
            applyButton.textContent = 'Apply Settings (Performance Boost)';
        } else {
            applyButton.style.background = 'linear-gradient(135deg, #3498db, #2980b9)';
            applyButton.textContent = 'Apply Settings';
        }
    }

    showNotification(message, type = 'info') {
        const notification = document.createElement('div');
        notification.className = `notification ${type}`;
        notification.textContent = message;
        notification.style.cssText = `
            position: fixed;
            top: 20px;
            right: 20px;
            padding: 1rem 2rem;
            border-radius: 8px;
            color: white;
            font-weight: 600;
            z-index: 1000;
            opacity: 0;
            transform: translateY(-20px);
            transition: all 0.3s ease;
        `;

        const colors = {
            success: '#27ae60',
            error: '#e74c3c',
            warning: '#f39c12',
            info: '#3498db'
        };

        notification.style.background = colors[type] || colors.info;
        
        document.body.appendChild(notification);
        
        // Animate in
        setTimeout(() => {
            notification.style.opacity = '1';
            notification.style.transform = 'translateY(0)';
        }, 100);
        
        // Remove after delay
        setTimeout(() => {
            notification.style.opacity = '0';
            notification.style.transform = 'translateY(-20px)';
            setTimeout(() => {
                document.body.removeChild(notification);
            }, 300);
        }, 3000);
    }
}

// Global functions for footer links
function exportConfig() {
    const config = {
        lodLevel: document.getElementById('lodLevel').value,
        shadowQuality: document.getElementById('shadowQuality').value,
        enableValidation: document.getElementById('enableValidation').checked,
        memoryBudget: document.getElementById('memoryBudget').value,
        timestamp: new Date().toISOString()
    };
    
    const dataStr = JSON.stringify(config, null, 2);
    const dataUri = 'data:application/json;charset=utf-8,'+ encodeURIComponent(dataStr);
    
    const exportFileDefaultName = `voxelvk-config-${Date.now()}.json`;
    
    const linkElement = document.createElement('a');
    linkElement.setAttribute('href', dataUri);
    linkElement.setAttribute('download', exportFileDefaultName);
    linkElement.click();
}

function loadDemo() {
    dashboard.showNotification('Demo mode not available in headless environment', 'warning');
}

function showAbout() {
    alert(`VoxelVK Elite ALL Graphics Engine v3.0.0

A high-performance Vulkan graphics engine built for real-time voxel rendering and advanced graphics techniques.

Features:
• Vulkan Memory Allocator (VMA) Integration
• Advanced Shader Pipeline
• Cascaded Shadow Mapping (CSM)
• Sparse Voxel Octrees (SVO)
• Level of Detail (LOD) System
• Real-time Performance Monitoring

Built with C++20, Vulkan 1.3, and modern graphics techniques.`);
}

// Initialize dashboard when DOM is loaded
let dashboard;
document.addEventListener('DOMContentLoaded', () => {
    dashboard = new VoxelVKDashboard();
});

// Export for global access
window.VoxelVKDashboard = VoxelVKDashboard;