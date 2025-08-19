
import json
import logging
import numpy as np
from pathlib import Path
from typing import Dict, Optional
from concurrent.futures import Future, ThreadPoolExecutor
from .rle import rle_encode, rle_decode

try:
    import zstandard as zstd
    has_zstd=True
except Exception:
    has_zstd=False
try:
    import lz4.frame as lz4f
    has_lz4=True
except Exception:
    has_lz4=False

class ChunkStore:
    def __init__(self, root="world_save", codec="zstd", use_rle=True, threads=4):
        self.root = Path(root)
        self.root.mkdir(parents=True, exist_ok=True)
        self.codec = codec
        self.use_rle = use_rle
        self.pool = ThreadPoolExecutor(max_workers=max(1, threads))
        self.futures: Dict[str, Future] = {}
        self.manifest_path = self.root / "manifest.json"
        if self.manifest_path.exists():
            try:
                self.manifest = json.loads(self.manifest_path.read_text())
            except Exception:
                self.manifest = {}
        else:
            self.manifest = {}

    def _region_dir(self, cx, cz):
        d = self.root / f"r.{cx//32}.{cz//32}"; d.mkdir(exist_ok=True); return d
    def chunk_path(self, cx, cz): return self._region_dir(cx, cz) / f"c.{cx}.{cz}.bin"

    def _compress(self, b: bytes) -> bytes:
        if self.codec=="zstd" and has_zstd:
            c = zstd.ZstdCompressor(level=10); return c.compress(b)
        if self.codec=="lz4" and has_lz4:
            return lz4f.compress(b, compression_level=9, block_size=lz4f.BLOCKSIZE_MAX1MB, content_checksum=True)
        return b

    def _decompress(self, b: bytes) -> bytes:
        if self.codec=="zstd" and has_zstd:
            d = zstd.ZstdDecompressor(); return d.decompress(b)
        if self.codec=="lz4" and has_lz4:
            return lz4f.decompress(b)
        return b

    def save_chunk_sync(self, chunk):
        try:
            cx, cz = chunk.position
            vox = chunk.voxels.astype(np.uint8)
            if self.use_rle:
                vals, counts, shape = rle_encode(vox)
                header = np.array([shape[0], shape[1], shape[2], vals.size, counts.size], dtype=np.int32).tobytes()
                payload = vals.tobytes() + counts.tobytes()
                data = header + payload
            else:
                header = np.array([vox.shape[0], vox.shape[1], vox.shape[2], 0, 0], dtype=np.int32).tobytes()
                data = header + vox.tobytes()
            blob = self._compress(data)
            path = self.chunk_path(cx, cz)
            path.write_bytes(blob)
            idx = self._region_dir(cx, cz) / "index.json"
            table = {}
            if idx.exists():
                try:
                    table = json.loads(idx.read_text())
                except Exception:
                    table = {}
            table[f"{cx},{cz}"] = {"saved": True}
            idx.write_text(json.dumps(table))
            region = f"r.{cx//32}.{cz//32}"
            entry = self.manifest.get(region, {"codec": self.codec, "chunks": {}})
            entry["codec"] = self.codec
            entry["chunks"][f"{cx},{cz}"] = {"offset": 0}
            self.manifest[region] = entry
            self.manifest_path.write_text(json.dumps(self.manifest))
        except Exception as exc:
            raise RuntimeError(f"failed to save chunk at {chunk.position}") from exc

    def save_async(self, chunk):
        """Schedule a chunk to be saved on a background thread.

        Callers must subsequently invoke :meth:`wait_all` and handle any
        exceptions raised during the save operations.
        """

        key = f"{chunk.position[0]},{chunk.position[1]}"
        self.futures[key] = self.pool.submit(self.save_chunk_sync, chunk)

    def load_chunk(self, cx, cz) -> Optional[Dict]:
        p = self.chunk_path(cx, cz)
        if not p.exists(): return None
        blob = p.read_bytes()
        raw = self._decompress(blob)
        header = np.frombuffer(raw[:20], dtype=np.int32)
        H, Y, S, nvals, ncnt = header.tolist()
        rest = raw[20:]
        if nvals>0:
            vals = np.frombuffer(rest[:nvals], dtype=np.uint8)
            counts = np.frombuffer(rest[nvals:nvals+4*ncnt], dtype=np.int32)
            vox = rle_decode(vals, counts, (H,Y,S))
        else:
            vox = np.frombuffer(rest, dtype=np.uint8).reshape((H,Y,S))
        return {"voxels": vox, "height": int(Y), "size": int(S)}

    def wait_all(self):
        errors = []
        for f in list(self.futures.values()):
            try:
                f.result(timeout=5)
            except Exception as exc:
                logging.exception("chunk save failed: %s", exc)
                errors.append(exc)
        self.futures.clear()
        self.pool.shutdown(wait=True)
        if errors:
            raise RuntimeError(f"{len(errors)} chunk saves failed") from errors[0]

    def delete_chunk(self, cx: int, cz: int) -> None:
        """Remove chunk data and update manifest/index."""
        region = f"r.{cx//32}.{cz//32}"
        reg_dir = self.root / region
        path = reg_dir / f"c.{cx}.{cz}.bin"
        if path.exists():
            path.unlink()
        entry = self.manifest.get(region)
        if entry and "chunks" in entry:
            entry["chunks"].pop(f"{cx},{cz}", None)
            if not entry["chunks"]:
                self.manifest.pop(region, None)
                reg_dir = self.root / region
                if reg_dir.exists():
                    for f in reg_dir.glob("*"):
                        f.unlink()
                    reg_dir.rmdir()
            self.manifest_path.write_text(json.dumps(self.manifest))
        idx = reg_dir / "index.json"
        if idx.exists():
            try:
                table = json.loads(idx.read_text())
            except Exception:
                table = {}
            table.pop(f"{cx},{cz}", None)
            if table:
                idx.write_text(json.dumps(table))
            else:
                idx.unlink()

    def compact(self) -> None:
        """Delete stray chunk files not referenced in the manifest."""
        for region, entry in list(self.manifest.items()):
            reg_dir = self.root / region
            live = set(entry.get("chunks", {}).keys())
            if reg_dir.exists():
                for chunk_file in reg_dir.glob("c.*.*.bin"):
                    coords = chunk_file.stem[2:].split(".")
                    key = f"{coords[0]},{coords[1]}"
                    if key not in live:
                        chunk_file.unlink()
            if not live and reg_dir.exists():
                for f in reg_dir.glob("*"):
                    f.unlink()
                reg_dir.rmdir()
                self.manifest.pop(region)
        self.manifest_path.write_text(json.dumps(self.manifest))
