#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <cstdio>
#include <string>
#include <vector>
#include <cstdint>

#include <fstream>
#include <zstd.h>

struct Vertex {
    float x, y, z;
};

int main(int argc, char** argv) {
    std::string chunk_file;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--chunk-file" && i + 1 < argc) {
            chunk_file = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::printf("Usage: %s [--chunk-file PATH]\n", argv[0]);
            return 0;
        }
    }

    std::vector<Vertex> vertices;
    if (!chunk_file.empty()) {
        std::ifstream ifs(chunk_file, std::ios::binary);
        std::vector<std::uint8_t> blob((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        unsigned long long raw_size = ZSTD_getFrameContentSize(blob.data(), blob.size());
        if (raw_size != ZSTD_CONTENTSIZE_ERROR && raw_size != ZSTD_CONTENTSIZE_UNKNOWN) {
            std::vector<std::uint8_t> raw(raw_size);
            size_t res = ZSTD_decompress(raw.data(), raw.size(), blob.data(), blob.size());
            if (!ZSTD_isError(res) && raw.size() >= 20) {
                const std::int32_t *header = reinterpret_cast<const std::int32_t*>(raw.data());
                int H = header[0];
                int Y = header[1];
                int S = header[2];
                int nvals = header[3];
                int ncnt = header[4];
                const std::uint8_t *ptr = raw.data() + 20;
                std::vector<std::uint8_t> vox;
                if (nvals > 0) {
                    std::vector<std::uint8_t> vals(ptr, ptr + nvals);
                    const std::int32_t *cnt_ptr = reinterpret_cast<const std::int32_t*>(ptr + nvals);
                    std::vector<std::int32_t> counts(cnt_ptr, cnt_ptr + ncnt);
                    vox.reserve(static_cast<std::size_t>(H) * Y * S);
                    for (std::size_t i = 0; i < vals.size(); ++i) {
                        vox.insert(vox.end(), counts[i], vals[i]);
                    }
                } else {
                    vox.assign(ptr, ptr + static_cast<std::size_t>(H) * Y * S);
                }
                for (int y = 0; y < H; ++y) {
                    for (int z = 0; z < S; ++z) {
                        for (int x = 0; x < S; ++x) {
                            std::uint8_t v = vox[(y * S + z) * S + x];
                            if (v) {
                                vertices.push_back({static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
                            }
                        }
                    }
                }
            }
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

    SDL_Window* window = SDL_CreateWindow("Voxel Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                         800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glctx = SDL_GL_CreateContext(window);
    if (!glctx) {
        std::fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }

        glViewport(0, 0, 800, 600);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glPointSize(4.0f);
        glBegin(GL_POINTS);
        for (const auto& v : vertices) {
            glVertex3f(v.x / 32.0f - 0.5f, v.y / 32.0f - 0.5f, v.z / 32.0f - 0.5f);
        }
        glEnd();

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(glctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

