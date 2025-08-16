#!/usr/bin/env python3
"""Utility to refresh vendor copies of external projects.

This script validates that the VULKEN-3D-WORLD submodule exists and can optionally
forcefully refresh any mirrored directories. It intentionally ignores VCS and build
artifacts to keep the tree clean.
"""
from __future__ import annotations
import argparse
import os
import pathlib
import shutil
import sys

def copytree(src: pathlib.Path, dst: pathlib.Path, force: bool) -> None:
    if dst.exists() and force:
        shutil.rmtree(dst)
    shutil.copytree(src, dst, ignore=shutil.ignore_patterns('.git', 'build', 'CMakeFiles', '__pycache__'))

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument('--force', action='store_true', help='overwrite existing files')
    args = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parents[1]
    ext = root / 'external' / 'VULKEN-3D-WORLD'
    if not (ext / 'CMakeLists.txt').exists():
        print('Missing external/VULKEN-3D-WORLD. Clone with:\n  gh repo clone user/VULKEN-3D-WORLD external/VULKEN-3D-WORLD', file=sys.stderr)
        return 1

    mirror = root / 'VULKEN-3D-WORLD.mirror'
    copytree(ext, mirror, args.force)
    print('Merged', ext, '->', mirror)
    return 0

if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except Exception as exc:  # pragma: no cover
        print(f'fATAL: {exc}', file=sys.stderr)
        raise SystemExit(1)
