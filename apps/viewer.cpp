#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <iostream>
#include <vector>

#include "world/persistence.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: viewer <world_dir>\n";
        return 1;
    }

    std::string root = argv[1];
    world::ChunkStore store(root, "zstd", true);
    auto chunk = store.load_chunk(0, 0);
    if (!chunk) {
        std::cerr << "Failed to load chunk at 0,0\n";
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow("viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       800, 600, SDL_WINDOW_OPENGL);
    if (!win) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    if (!ctx) {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glPointSize(4.0f);

    bool running = true;
    float angle = 0.0f;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
        }

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60.0, 800.0 / 600.0, 0.1, 1000.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(-chunk->size / 2.0f, -chunk->height / 2.0f, -chunk->size * 2.0f);
        glRotatef(angle, 0.0f, 1.0f, 0.0f);
        angle += 0.5f;

        glBegin(GL_POINTS);
        const auto& vox = chunk->voxels;
        int S = chunk->size;
        int H = chunk->height;
        for (int y = 0; y < H; ++y) {
            for (int z = 0; z < S; ++z) {
                for (int x = 0; x < S; ++x) {
                    std::size_t idx = x + z * S + y * S * S;
                    std::uint8_t v = vox[idx];
                    if (v) {
                        glColor3ub(v, v, v);
                        glVertex3f(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
                    }
                }
            }
        }
        glEnd();

        SDL_GL_SwapWindow(win);
        SDL_Delay(16);
    }

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
