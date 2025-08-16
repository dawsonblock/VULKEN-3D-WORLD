#!/usr/bin/env python3
import argparse
import shutil
from pathlib import Path
import sys

IGNORE_PATTERNS = shutil.ignore_patterns('.git', 'build', 'build-*', '__pycache__')

def copytree(src: Path, dst: Path, force: bool = False) -> None:
    if dst.exists():
        if force:
            shutil.rmtree(dst)
        else:
            print(f"Destination {dst} exists. Use --force to overwrite", file=sys.stderr)
            sys.exit(1)
    shutil.copytree(src, dst, ignore=IGNORE_PATTERNS)


def main() -> None:
    parser = argparse.ArgumentParser(description="Merge external snapshots into the tree")
    parser.add_argument('src', help='source directory')
    parser.add_argument('dst', help='destination directory')
    parser.add_argument('--force', action='store_true', help='overwrite destination if it exists')
    args = parser.parse_args()

    external = Path('external/VULKEN-3D-WORLD/CMakeLists.txt')
    if not external.exists():
        print("Missing external/VULKEN-3D-WORLD. Clone with:\n  gh repo clone VULKEN-3D/VULKEN-3D-WORLD external/VULKEN-3D-WORLD", file=sys.stderr)
        sys.exit(1)

    src = Path(args.src)
    src = Path(args.src)
    dst = Path(args.dst)

    # Only check for external/VULKEN-3D-WORLD/CMakeLists.txt if src or dst is (or is within) external/VULKEN-3D-WORLD
    external_dir = Path('external/VULKEN-3D-WORLD')
    if (external_dir in src.parents or src.resolve() == external_dir.resolve() or
        external_dir in dst.parents or dst.resolve() == external_dir.resolve()):
        external_cmake = external_dir / 'CMakeLists.txt'
        if not external_cmake.exists():
            print("Missing external/VULKEN-3D-WORLD. Clone with:\n  gh repo clone VULKEN-3D/VULKEN-3D-WORLD external/VULKEN-3D-WORLD", file=sys.stderr)
            sys.exit(1)
    if not src.exists():
        print(f"Source {src} does not exist", file=sys.stderr)
        sys.exit(1)
    try:
        copytree(src, dst, force=args.force)
    except Exception as exc:
        print(f"Merge failed: {exc}", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()
