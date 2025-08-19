#!/usr/bin/env python3
"""Validate GLSL shaders with glslangValidator.

Walks ``shaders`` and ``shaders_vk`` directories and runs
``glslangValidator`` on every shader file. Any warning or error output
causes the script to exit with a non-zero status.
"""
from __future__ import annotations

import pathlib
import subprocess
import sys
from typing import Iterable

# Known shader stages supported by glslangValidator
STAGES = {
    "vert",
    "frag",
    "comp",
    "geom",
    "tesc",
    "tese",
    "mesh",
    "task",
    "rgen",
    "rchit",
    "rmiss",
    "rcall",
}


def iter_shader_files(dirs: Iterable[pathlib.Path]) -> Iterable[pathlib.Path]:
    """Yield shader files from the provided directories."""
    for directory in dirs:
        if not directory.exists():
            continue
        for path in directory.rglob("*"):
            if path.is_file() and get_stage(path) is not None:
                yield path


def get_stage(path: pathlib.Path) -> str | None:
    """Return the shader stage for ``path`` or ``None`` if unknown."""
    name_parts = path.name.split(".")
    if len(name_parts) >= 2:
        stage = name_parts[-2]
        if stage in STAGES:
            return stage
    suffix = path.suffix.lstrip(".")
    if suffix in STAGES:
        return suffix
    return None


def validate(path: pathlib.Path, stage: str) -> None:
    """Run glslangValidator on ``path`` for the given ``stage``."""
    cmd = [
        "glslangValidator",
        "--quiet",
        "--auto-map-locations",
        "-V",
        "-S",
        stage,
        str(path),
    ]
    proc = subprocess.run(cmd, capture_output=True, text=True)
    output = proc.stdout + proc.stderr
    if proc.returncode != 0 or "WARNING" in output.upper():
        sys.stdout.write(proc.stdout)
        sys.stderr.write(proc.stderr)
        raise RuntimeError(f"glslangValidator failed for {path}")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    shader_dirs = [root / "shaders", root / "shaders_vk"]
    errors = 0
    for shader in iter_shader_files(shader_dirs):
        stage = get_stage(shader)
        try:
            validate(shader, stage)  # type: ignore[arg-type]
            print(f"Validated {shader.relative_to(root)}")
        except RuntimeError as exc:
            print(exc, file=sys.stderr)
            errors += 1
    return 1 if errors else 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:  # pragma: no cover
        print(f"FATAL: {exc}", file=sys.stderr)
        raise SystemExit(1)
