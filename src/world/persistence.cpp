#include "persistence.hpp"

#include <fstream>
#include <stdexcept>
#include <iterator>
#include <algorithm>
#include <iostream>

#if __has_include(<zstd.h>)
#define VOXELVK_HAS_ZSTD 1
#include <zstd.h>
#else
#define VOXELVK_HAS_ZSTD 0
#endif

#if __has_include(<lz4frame.h>)
#define VOXELVK_HAS_LZ4 1
#include <lz4frame.h>
#else
#define VOXELVK_HAS_LZ4 0
#endif

namespace world {

ChunkStore::ChunkStore(const std::string &root, const std::string &codec, bool use_rle, int threads)
    : root_(root), codec_(codec), use_rle_(use_rle), thread_count_(threads) {
    std::filesystem::create_directories(root_);
}

ChunkStore::~ChunkStore() { wait_all(); }

std::filesystem::path ChunkStore::_region_dir(int cx, int cz) const {
    std::filesystem::path d = root_ / ("r." + std::to_string(cx / 32) + "." + std::to_string(cz / 32));
    std::filesystem::create_directories(d);
    return d;
}

std::filesystem::path ChunkStore::chunk_path(int cx, int cz) const {
    return _region_dir(cx, cz) / ("c." + std::to_string(cx) + "." + std::to_string(cz) + ".bin");
}

void ChunkStore::rle_encode(const std::vector<std::uint8_t> &arr,
                            std::vector<std::uint8_t> &vals,
                            std::vector<std::int32_t> &counts) {
    vals.clear();
    counts.clear();
    if (arr.empty()) return;
    std::uint8_t prev = arr[0];
    std::int32_t cnt = 1;
    for (std::size_t i = 1; i < arr.size(); ++i) {
        std::uint8_t v = arr[i];
        if (v == prev) {
            ++cnt;
        } else {
            vals.push_back(prev);
            counts.push_back(cnt);
            prev = v;
            cnt = 1;
        }
    }
    vals.push_back(prev);
    counts.push_back(cnt);
}

std::vector<std::uint8_t> ChunkStore::rle_decode(const std::vector<std::uint8_t> &vals,
                                                  const std::vector<std::int32_t> &counts,
                                                  std::size_t total) {
    std::vector<std::uint8_t> out;
    out.reserve(total);
    for (std::size_t i = 0; i < vals.size(); ++i) {
        out.insert(out.end(), counts[i], vals[i]);
    }
    return out;
}

std::vector<std::uint8_t> ChunkStore::_compress(const std::vector<std::uint8_t> &data) const {
    if (codec_ == "zstd" && VOXELVK_HAS_ZSTD) {
        size_t max_size = ZSTD_compressBound(data.size());
        std::vector<std::uint8_t> out(max_size);
        size_t csize = ZSTD_compress(out.data(), max_size, data.data(), data.size(), 10);
        if (ZSTD_isError(csize)) {
            throw std::runtime_error("zstd compress failed");
        }
        out.resize(csize);
        return out;
    }
#if VOXELVK_HAS_LZ4
    if (codec_ == "lz4") {
        size_t max_size = LZ4F_compressFrameBound(data.size(), nullptr);
        std::vector<std::uint8_t> out(max_size);
        size_t csize = LZ4F_compressFrame(out.data(), max_size, data.data(), data.size(), nullptr);
        if (LZ4F_isError(csize)) {
            throw std::runtime_error("lz4 compress failed");
        }
        out.resize(csize);
        return out;
    }
#endif
    return data;
}

std::vector<std::uint8_t> ChunkStore::_decompress(const std::vector<std::uint8_t> &data) const {
    if (codec_ == "zstd" && VOXELVK_HAS_ZSTD) {
        unsigned long long size = ZSTD_getFrameContentSize(data.data(), data.size());
        if (size == ZSTD_CONTENTSIZE_ERROR || size == ZSTD_CONTENTSIZE_UNKNOWN) {
            throw std::runtime_error("zstd frame size unknown");
        }
        std::vector<std::uint8_t> out(size);
        size_t dsize = ZSTD_decompress(out.data(), size, data.data(), data.size());
        if (ZSTD_isError(dsize)) {
            throw std::runtime_error("zstd decompress failed");
        }
        return out;
    }
#if VOXELVK_HAS_LZ4
    if (codec_ == "lz4") {
        LZ4F_dctx *dctx;
        LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);
        LZ4F_frameInfo_t info{};
        size_t consumed = 0;
        size_t src_size = data.size();
        const uint8_t *src = data.data();
        LZ4F_getFrameInfo(dctx, &info, src, &consumed);
        src += consumed;
        src_size -= consumed;
        if (info.contentSize == 0) {
            LZ4F_freeDecompressionContext(dctx);
            throw std::runtime_error("lz4 unknown content size");
        }
        std::vector<std::uint8_t> out(info.contentSize);
        size_t dst_size = out.size();
        size_t result = LZ4F_decompress(dctx, out.data(), &dst_size, src, &src_size, nullptr);
        LZ4F_freeDecompressionContext(dctx);
        if (LZ4F_isError(result)) {
            throw std::runtime_error("lz4 decompress failed");
        }
        return out;
    }
#endif
    return data;
}

void ChunkStore::save_chunk_sync(const Chunk &chunk) {
    int cx = chunk.position.first;
    int cz = chunk.position.second;
    const auto &vox = chunk.voxels;
    if (static_cast<std::size_t>(chunk.size * chunk.height * chunk.size) != vox.size()) {
        throw std::runtime_error("voxel array size mismatch");
    }
    std::vector<std::uint8_t> vals;
    std::vector<std::int32_t> counts;
    std::vector<std::uint8_t> data;

    std::int32_t header[5] = {chunk.height, chunk.size, chunk.size, 0, 0};
    if (use_rle_) {
        rle_encode(vox, vals, counts);
        header[3] = static_cast<std::int32_t>(vals.size());
        header[4] = static_cast<std::int32_t>(counts.size());
    }

    data.insert(data.end(), reinterpret_cast<std::uint8_t *>(header),
                reinterpret_cast<std::uint8_t *>(header) + sizeof(header));
    if (use_rle_) {
        data.insert(data.end(), vals.begin(), vals.end());
        data.insert(data.end(), reinterpret_cast<const std::uint8_t *>(counts.data()),
                    reinterpret_cast<const std::uint8_t *>(counts.data()) + counts.size() * sizeof(std::int32_t));
    } else {
        data.insert(data.end(), vox.begin(), vox.end());
    }
    auto blob = _compress(data);
    auto path = chunk_path(cx, cz);
    std::ofstream ofs(path, std::ios::binary);
    ofs.write(reinterpret_cast<const char *>(blob.data()), static_cast<std::streamsize>(blob.size()));
    ofs.close();

    // Minimal index.json write
    auto idx = _region_dir(cx, cz) / "index.json";
    std::ofstream idxf(idx, std::ios::binary);
    idxf << "{\"" << cx << "," << cz << "\": {\"saved\": true}}";
}

void ChunkStore::save_async(const Chunk &chunk) {
    futures_.emplace_back(std::async(std::launch::async, [this, chunk] { save_chunk_sync(chunk); }));
}

std::optional<ChunkData> ChunkStore::load_chunk(int cx, int cz) {
    auto path = chunk_path(cx, cz);
    if (!std::filesystem::exists(path)) return std::nullopt;
    std::ifstream ifs(path, std::ios::binary);
    std::vector<std::uint8_t> blob((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();
    auto raw = _decompress(blob);
    if (raw.size() < sizeof(std::int32_t) * 5) return std::nullopt;
    const std::int32_t *header = reinterpret_cast<const std::int32_t *>(raw.data());
    int height = header[0];
    int size_x = header[1];
    int size_z = header[2];
    int nvals = header[3];
    int ncnt = header[4];
    std::size_t total = static_cast<std::size_t>(height) * size_x * size_z;
    const std::uint8_t *ptr = raw.data() + sizeof(std::int32_t) * 5;
    std::vector<std::uint8_t> vox;
    if (nvals > 0) {
        if (raw.size() < sizeof(std::int32_t) * 5 + static_cast<std::size_t>(nvals) + static_cast<std::size_t>(ncnt) * sizeof(std::int32_t)) {
            return std::nullopt;
        }
        std::vector<std::uint8_t> vals(ptr, ptr + nvals);
        const std::int32_t *cnt_ptr = reinterpret_cast<const std::int32_t *>(ptr + nvals);
        std::vector<std::int32_t> counts(cnt_ptr, cnt_ptr + ncnt);
        vox = rle_decode(vals, counts, total);
    } else {
        if (raw.size() < sizeof(std::int32_t) * 5 + total) return std::nullopt;
        vox.assign(ptr, ptr + total);
    }
    return ChunkData{std::move(vox), height, size_x};
}

void ChunkStore::wait_all() {
    for (auto &f : futures_) {
        try {
            f.get();
        } catch (const std::exception &e) {
            std::cerr << "[ChunkStore::wait_all] Exception in async save: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "[ChunkStore::wait_all] Unknown exception in async save." << std::endl;
        }
    }
    futures_.clear();
}

} // namespace world

