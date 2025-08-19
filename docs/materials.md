# Material Configuration

Material properties are described in `assets/config/materials.json`. Each entry
provides constant PBR parameters that can be accessed by render passes via the
material manager.

```json
{
  "materials": {
    "GRASS": {
      "albedo": [0.22, 0.35, 0.12],
      "metallic": 0.0,
      "roughness": 0.9
    }
  }
}
```

Schema:

- `materials`: object mapping material names to their properties.
- `albedo`: array of three floating point values representing linear RGB color.
- `metallic`: float in `[0,1]` describing metalness.
- `roughness`: float in `[0,1]` controlling surface microfacet roughness.

The `MaterialManager` parses this file and uploads the values to a uniform
buffer bound in descriptor sets so shading passes can fetch material parameters.
