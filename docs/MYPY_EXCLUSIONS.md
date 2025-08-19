# Mypy Exclusions

The project's `mypy.ini` excludes certain paths from type checking using the pattern
`exclude = ^(src/physics/|tests/)`.

- `src/physics/` – physics routines not yet typed.
- `tests/` – all test modules, including `tests/test_capsule_ground.py`.

These directories are skipped during type checking to keep the build fast while these areas
are still under development.
