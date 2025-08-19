#include "persistence.hpp"

#include <fstream>
#include <stdexcept>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <sstream>

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

std::filesystem::path ChunkStore::_manifest_path(int cx, int cz) const {
    return _region_dir(cx, cz) / "region_manifest.json";
}

static std::unordered_map<std::string, std::size_t>
_load_manifest(const std::filesystem::path &p) {
    std::unordered_map<std::string, std::size_t> table;
    if (!std::filesystem::exists(p)) return table;
    std::ifstream ifs(p);
    if (!ifs) return table;
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    std::size_t pos = 0;
    while ((pos = content.find('"', pos)) != std::string::npos) {
        std::size_t end = content.find('"', pos + 1);
        if (end == std::string::npos) break;
        std::string key = content.substr(pos + 1, end - pos - 1);
        std::size_t size_pos = content.find("\"size\"", end);
        if (size_pos == std::string::npos) break;
        size_pos = content.find(':', size_pos);
        if (size_pos == std::string::npos) break;
        std::size_t comma = content.find_first_of(",}", size_pos);
        std::size_t value = static_cast<std::size_t>(std::stoll(content.substr(size_pos + 1, comma - size_pos - 1)));
        table[key] = value;
        pos = comma;
    }
    return table;
}

static void _write_manifest(const std::filesystem::path &p,
                            const std::unordered_map<std::string, std::size_t> &table) {
    std::ofstream ofs(p, std::ios::binary);
    ofs << '{';
    bool first = true;
    for (const auto &kv : table) {
        if (!first) ofs << ',';
        ofs << '"' << kv.first << "\": {\"size\": " << kv.second << '}';
        first = false;
    }
    ofs << '}';
}

void ChunkStore::_update_manifest(int cx, int cz, std::size_t size) {
    auto path = _manifest_path(cx, cz);
    auto table = _load_manifest(path);
    table[std::to_string(cx) + "," + std::to_string(cz)] = size;
    _write_manifest(path, table);
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
        LZ4F_preferences_t prefs{};
        prefs.frameInfo.contentSize = data.size();
        size_t max_size = LZ4F_compressFrameBound(data.size(), &prefs);
        std::vector<std::uint8_t> out(max_size);
        size_t csize = LZ4F_compressFrame(out.data(), max_size, data.data(), data.size(), &prefs);
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
        std::vector<std::uint8_t> out;
        out.resize(info.contentSize ? info.contentSize : data.size() * 4);
        size_t pos = 0;
        while (true) {
            size_t dst_size = out.size() - pos;
            size_t ret = LZ4F_decompress(dctx, out.data() + pos, &dst_size, src, &src_size, nullptr);
            if (LZ4F_isError(ret)) {
                LZ4F_freeDecompressionContext(dctx);
                throw std::runtime_error("lz4 decompress failed");
            }
            pos += dst_size;
            if (ret == 0) break;
            if (pos == out.size()) out.resize(out.size() * 2);
        }
        LZ4F_freeDecompressionContext(dctx);
        out.resize(pos);
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
    if (use_rle_) {
        rle_encode(vox, vals, counts);
        std::int32_t header[5] = {chunk.size, chunk.height, chunk.height,
                                  static_cast<std::int32_t>(vals.size()),
                                  static_cast<std::int32_t>(counts.size())};
        data.insert(data.end(), reinterpret_cast<std::uint8_t *>(header),
                    reinterpret_cast<std::uint8_t *>(header) + 20);
        data.insert(data.end(), vals.begin(), vals.end());
        data.insert(data.end(), reinterpret_cast<const std::uint8_t *>(counts.data()),
                    reinterpret_cast<const std::uint8_t *>(counts.data()) + counts.size() * sizeof(std::int32_t));
    } else {
        std::int32_t header[5] = {chunk.height, chunk.size, chunk.size, 0, 0};
        data.insert(data.end(), reinterpret_cast<std::uint8_t *>(header),
                    reinterpret_cast<std::uint8_t *>(header) + 20);
        data.insert(data.end(), vox.begin(), vox.end());
    }
    auto blob = _compress(data);
    auto path = chunk_path(cx, cz);
    std::ofstream ofs(path, std::ios::binary);
    ofs.write(reinterpret_cast<const char *>(blob.data()), static_cast<std::streamsize>(blob.size()));
    ofs.close();
    _update_manifest(cx, cz, blob.size());
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
    if (raw.size() < 20) return std::nullopt;
    const std::int32_t *header = reinterpret_cast<const std::int32_t *>(raw.data());
    int H = header[0];
    int Y = header[1];
    int S = header[2];
    int nvals = header[3];
    int ncnt = header[4];
    const std::uint8_t *ptr = raw.data() + 20;
    std::vector<std::uint8_t> vox;
    if (nvals > 0) {
        std::vector<std::uint8_t> vals(ptr, ptr + nvals);
        const std::int32_t *cnt_ptr = reinterpret_cast<const std::int32_t *>(ptr + nvals);
        std::vector<std::int32_t> counts(cnt_ptr, cnt_ptr + ncnt);
        vox = rle_decode(vals, counts, static_cast<std::size_t>(H) * Y * S);
    } else {
        vox.assign(ptr, ptr + static_cast<std::size_t>(H) * Y * S);
    }
    return ChunkData{std::move(vox), Y, S};
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

void ChunkStore::compact_region(int rx, int rz) {
    auto dir = root_ / ("r." + std::to_string(rx) + "." + std::to_string(rz));
    auto manifest = dir / "region_manifest.json";
    auto table = _load_manifest(manifest);
    bool changed = false;
    for (auto it = table.begin(); it != table.end();) {
        auto key = it->first;
        auto pos = key.find(',');
        int cx = std::stoi(key.substr(0, pos));
        int cz = std::stoi(key.substr(pos + 1));
        auto chunk_file = dir / ("c." + std::to_string(cx) + "." + std::to_string(cz) + ".bin");
        if (!std::filesystem::exists(chunk_file)) {
            it = table.erase(it);
            changed = true;
        } else {
            ++it;
        }
    }
    if (changed) {
        _write_manifest(manifest, table);
    }
}

} // namespace world

