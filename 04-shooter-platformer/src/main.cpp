#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

struct SDLState {
    SDL_Window *window;
    SDL_Renderer *renderer;
};

void cleanup(SDLState &state);

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    SDLState state;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    const float width = 800;
    const float height = 640;
    state.window = SDL_CreateWindow("My Game", width, height, 0);

    if (!state.window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        cleanup(state);
        return 1;
    }

    state.renderer = SDL_CreateRenderer(state.window, NULL);
    if (!state.renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        cleanup(state);
        return 1;
    }

    SDL_Texture *idleTexture = IMG_LoadTexture(state.renderer, "./assets/light/Idle.png");

    if (!idleTexture) {
        std::cerr << "IMG_LoadTexture failed: " << SDL_GetError() << std::endl;
        cleanup(state);
        return 1;
    }

    bool running = true;
    SDL_Event event;

    std::cout << "Window created successfully. Press ESC or close window to exit." << std::endl;
    while (running) {

        // first check for events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }

        // handle the events (update)
        //
        // perform drawing commands at last

        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
        SDL_RenderClear(state.renderer);

        SDL_FRect srcRect = {0, 0, (float)idleTexture->w / 7, (float)idleTexture->h};
        SDL_RenderTexture(state.renderer, idleTexture, &srcRect, NULL);

        // swab buffers and present
        SDL_RenderPresent(state.renderer);
    }

    SDL_DestroyTexture(idleTexture);
    cleanup(state);
    return 0;
}

void cleanup(SDLState &state) {
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}
