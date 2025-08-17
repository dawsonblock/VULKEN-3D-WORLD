#!/usr/bin/env python3
"""Export voxel data to an interactive three.js scene.

The script expects a ``.npy`` file containing a three dimensional ``numpy`` array
where non-zero entries represent voxels. The generated HTML file embeds
``three.js`` from a CDN and renders a simple cube at each voxel position.
"""
from __future__ import annotations

import argparse
from pathlib import Path

import numpy as np
from jinja2 import Template


HTML_TEMPLATE = Template(
    """
<!DOCTYPE html>
<html>
  <head>
    <meta charset=\"utf-8\">
    <title>VULKEN Voxel Export</title>
    <script src=\"https://cdn.jsdelivr.net/npm/three@0.160/build/three.min.js\"></script>
  </head>
  <body>
    <script>
      const scene = new THREE.Scene();
      const camera = new THREE.PerspectiveCamera(75, window.innerWidth/window.innerHeight, 0.1, 1000);
      const renderer = new THREE.WebGLRenderer();
      renderer.setSize(window.innerWidth, window.innerHeight);
      document.body.appendChild(renderer.domElement);

      const geometry = new THREE.BoxGeometry(1, 1, 1);
      const material = new THREE.MeshNormalMaterial();
      const voxels = {{ positions | tojson }};
      for (const p of voxels) {
        const mesh = new THREE.Mesh(geometry, material);
        mesh.position.set(p[0], p[1], p[2]);
        scene.add(mesh);
      }

      camera.position.z = {{ camera_z }};
      function animate() {
        requestAnimationFrame(animate);
        renderer.render(scene, camera);
      }
      animate();
    </script>
  </body>
</html>
"""
)


def export_voxels(voxels: np.ndarray, output: Path) -> None:
    """Render *voxels* to an HTML file at *output*."""
    positions = np.argwhere(voxels)
    html = HTML_TEMPLATE.render(positions=positions.tolist(), camera_z=max(voxels.shape) * 2)
    output.write_text(html, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path, help=".npy file containing voxel data")
    parser.add_argument("output", type=Path, help="output HTML file")
    args = parser.parse_args()

    voxels = np.load(args.input)
    export_voxels(voxels, args.output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
