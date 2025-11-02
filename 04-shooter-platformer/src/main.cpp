#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_scancode.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_timer.h"
#include "SDL3/SDL_video.h"
#include "animation.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3_image/SDL_image.h>
#include <cstdint>
#include <iostream>
#include <vector>

struct SDLState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    // logW and logH means logical width/height it's used by
    // SDL_SetRenderLogicalPresentation to make a logical width of our game without us
    // having to worry about window resize and screen resolution, width and height here
    // are the actual widnow size on the client
    int width, height, logW, logH;
};

bool initialize(SDLState &state);
void cleanup(SDLState &state);

struct Resources {
    const int ANIM_PLAYER_IDLE = 0;
    std::vector<Animation> playerAnims;
    std::vector<SDL_Texture *> textures;
    SDL_Texture *idleTexture;

    SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string &filePath) {
        SDL_Texture *texture = IMG_LoadTexture(renderer, filePath.c_str());
        // this makes the sprite to be streched without "antialiasing"
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

        if (!texture) {
            std::cerr << "IMG_LoadTexture failed: " << filePath << SDL_GetError()
                      << std::endl;
            return texture;
        }
        textures.push_back(texture);

        return texture;
    }

    void load(SDLState &state) {
        playerAnims.resize(5);
        playerAnims[ANIM_PLAYER_IDLE] = Animation(7, 1);
        idleTexture = loadTexture(state.renderer, "./assets/light/Idle.png");
    }
    void unload() {
        for (auto *texture : textures) {
            SDL_DestroyTexture(texture);
        }
    }
};

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    SDLState state;

    if (!initialize(state)) {
        std::cerr << "Failed to initialize: " << SDL_GetError() << std::endl;
        return 1;
    }

    // load game assets
    Resources res;
    res.load(state);
    float spriteSize = (float)res.idleTexture->w / 7;

    // setup game data
    // keys is used to know which keys are being pressed in our program
    const bool *keys = SDL_GetKeyboardState(NULL);
    float playerX = 0;
    bool flipHorizontal = false;

    // first usage of logH, we actually know where the floor logically is without having
    // to care about the window size
    const float floor = state.logH;

    uint64_t previousTime = SDL_GetTicks();

    std::cout << "Window created successfully. Press ESC or close window to exit."
              << std::endl;
    bool running = true;
    SDL_Event event;
    while (running) {
        uint64_t now = SDL_GetTicks();
        // note get ticks is in milisecond and we want to work with seconds in this
        // engine, so we divide by a thousand, because 1000ms = 1s
        float deltaTime = (now - previousTime) / 1000.0f;
        previousTime = now;

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
        // no else if because if the two keys are pressed at once the character will stand
        // still
        if (keys[SDL_SCANCODE_A]) {
            flipHorizontal = true;
            playerVelocity += -75.0f;
        }
        if (keys[SDL_SCANCODE_D]) {
            flipHorizontal = false;
            playerVelocity += 75.0f;
        }
        playerX += playerVelocity * deltaTime;

        // perform drawing commands at last

        SDL_SetRenderDrawColor(state.renderer, 20, 0, 0, 255);
        SDL_RenderClear(state.renderer);

        SDL_FRect srcRect = {0, 0, spriteSize, spriteSize};
        SDL_FRect destRect = {playerX, floor - srcRect.h, spriteSize, spriteSize};
        SDL_RenderTextureRotated(
            state.renderer,
            res.idleTexture,
            &srcRect,
            &destRect,
            0,
            NULL,
            flipHorizontal ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

        // swab buffers and present
        SDL_RenderPresent(state.renderer);
    }

    res.unload();
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
    state.window =
        SDL_CreateWindow("My Game", state.width, state.height, SDL_WINDOW_RESIZABLE);

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

    // configure presentation, this makes the game be rendered at a logical size that we
    // define, so that the game is scaled accordingly without us having to worry about
    // scaling objects, it also makes the size be respected without caring about the real
    // window size
    //
    // In short: allows us to work in a resolution independent from the window size /
    // monitor resolution PRESENTATION_LETTERBOX = "create black bars around the game"
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
