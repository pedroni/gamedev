#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_scancode.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_video.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

struct SDLState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    // logW and logH means logical width/height it's used by SDL_SetRenderLogicalPresentation to
    // make a logical width of our game without us having to worry about window resize and screen
    // resolution, width and height here are the actual widnow size on the client
    int width, height, logW, logH;
};

bool initialize(SDLState &state);
void cleanup(SDLState &state);

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    SDLState state;

    if (!initialize(state)) {
        std::cerr << "Failed to initialize: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Texture *idleTexture = IMG_LoadTexture(state.renderer, "./assets/light/Idle.png");
    // this makes the sprite to be streched without "antialiasing"
    SDL_SetTextureScaleMode(idleTexture, SDL_SCALEMODE_NEAREST);

    if (!idleTexture) {
        std::cerr << "IMG_LoadTexture failed: " << SDL_GetError() << std::endl;
        cleanup(state);
        return 1;
    }

    float spriteSize = (float)idleTexture->w / 7;

    // setup game data
    // keys is used to know which keys are being pressed in our program
    const bool *keys = SDL_GetKeyboardState(NULL);
    float playerX = 0;

    // first usage of logH, we actually know where the floor logically is without having to care
    // about the window size
    const float floor = state.logH;

    bool running = true;
    SDL_Event event;

    std::cout << "Window created successfully. Press ESC or close window to exit." << std::endl;
    while (running) {

        // first check for events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT: {
                running = false;
                break;
            }
            case SDL_EVENT_KEY_DOWN: {
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }
                break;
            }
            case SDL_EVENT_WINDOW_RESIZED: {
                state.width = event.window.data1;
                state.height = event.window.data2;
                break;
            }
            }
        }

        // handle the events (update)
        // if not pressing sets to 0
        float playerVelocity = 0;
        // no else if because if the two keys are pressed at once the character will stand still
        if (keys[SDL_SCANCODE_A]) {
            playerVelocity += -75.0f;
        }
        if (keys[SDL_SCANCODE_D]) {
            playerVelocity += 75.0f;
        }
        playerX += playerVelocity;

        // perform drawing commands at last

        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
        SDL_RenderClear(state.renderer);

        SDL_FRect srcRect = {0, 0, spriteSize, spriteSize};
        SDL_FRect destRect = {playerX, floor - srcRect.h, spriteSize, spriteSize};
        SDL_RenderTexture(state.renderer, idleTexture, &srcRect, &destRect);

        // swab buffers and present
        SDL_RenderPresent(state.renderer);
    }

    SDL_DestroyTexture(idleTexture);
    cleanup(state);
    return 0;
}

bool initialize(SDLState &state) {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    state.width = 800;
    state.height = 640;
    state.window = SDL_CreateWindow("My Game", state.width, state.height, SDL_WINDOW_RESIZABLE);

    if (!state.window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        cleanup(state);
        return false;
    }

    state.renderer = SDL_CreateRenderer(state.window, NULL);
    if (!state.renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        cleanup(state);
        return false;
    }

    // configure presentation, this makes the game be rendered at a logical size that we define, so
    // that the game is scaled accordingly without us having to worry about scaling objects, it also
    // makes the size be respected without caring about the real window size
    //
    // In short: allows us to work in a resolution independent from the window size / monitor
    // resolution PRESENTATION_LETTERBOX = "create black bars around the game"
    //
    state.logW = 640;
    state.logH = 320;
    SDL_SetRenderLogicalPresentation(
        state.renderer,
        state.logW,
        state.logH,
        SDL_LOGICAL_PRESENTATION_LETTERBOX);
    return true;
}

void cleanup(SDLState &state) {
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}
