# Type Checking Exclusions

The project omits certain modules from static type checking to keep the
build manageable:

- `src/physics/` – physics routines are experimental and rely heavily on
  dynamic numerical code that lacks annotations.
- `tests/` and `tests/test_capsule_ground.py` – test modules include
  rapid-prototype helpers and mock-heavy code that are not yet typed.

These exclusions reduce mypy noise while annotations are added over time.
