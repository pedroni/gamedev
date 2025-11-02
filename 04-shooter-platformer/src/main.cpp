#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_scancode.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_timer.h"
#include "SDL3/SDL_video.h"
#include "animation.h"
#include "gameobject.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3_image/SDL_image.h>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <ios>
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

    const bool *keys;
    SDLState() { keys = SDL_GetKeyboardState(NULL); }
};

const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_CHARACTERS = 1;
const int MAX_LAYERS = 2;

struct GameState {
    std::array<std::vector<GameObject>, MAX_LAYERS> layers;

    // so we know where the placer is at
    int playerIndex;

    GameState() {
        playerIndex = 0; // will change automatically on map loading
    }
};

struct Resources {
    const int ANIM_PLAYER_IDLE = 0;
    const int ANIM_PLAYER_WALK = 1;
    const int ANIM_PLAYER_RUN = 2;

    std::vector<Animation> playerAnims;
    std::vector<SDL_Texture *> textures;
    SDL_Texture *idleTexture, *runTexture, *walkTexture;

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
        playerAnims[ANIM_PLAYER_WALK] = Animation(7, 0.5);
        playerAnims[ANIM_PLAYER_RUN] = Animation(8, 0.5);

        idleTexture = loadTexture(state.renderer, "./assets/light/Idle.png");
        runTexture = loadTexture(state.renderer, "./assets/light/Run.png");
        walkTexture = loadTexture(state.renderer, "./assets/light/Walk.png");
    }
    void unload() {
        for (auto *texture : textures) {
            SDL_DestroyTexture(texture);
        }
    }
};

bool initialize(SDLState &state);
void cleanup(SDLState &state);
void update(
    const SDLState &state,
    GameState &gs,
    Resources &res,
    GameObject &obj,
    float deltaTime);
void drawObject(const SDLState &state, GameState &gs, GameObject &obj, float deltaTime);

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

    // setup game data
    // keys is used to know which keys are being pressed in our program
    GameState gs;

    // create the player
    GameObject player;
    player.type = ObjectType::PLAYER;
    player.data.player = PlayerData();
    player.texture = res.idleTexture;
    player.animations = res.playerAnims;
    player.currentAnimation = res.ANIM_PLAYER_IDLE;
    // when pressing the "acelerador" do carro ele acelera 300
    player.acceleration = glm::vec2(300, 0);
    player.maxSpeedX = 100;
    gs.layers[LAYER_IDX_CHARACTERS].push_back(player);

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
        for (std::vector<GameObject> &layer : gs.layers) {
            for (GameObject &obj : layer) {
                update(state, gs, res, obj, deltaTime);

                // step animation
                if (obj.currentAnimation != -1) {
                    obj.animations[obj.currentAnimation].step(deltaTime);
                }
            }
        }

        // perform drawing commands at last
        SDL_SetRenderDrawColor(state.renderer, 20, 0, 0, 255);
        SDL_RenderClear(state.renderer);

        // draw all objects
        for (std::vector<GameObject> &layer : gs.layers) {
            for (GameObject &obj : layer) {
                drawObject(state, gs, obj, deltaTime);
            }
        }

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

void drawObject(const SDLState &state, GameState &gs, GameObject &obj, float deltaTime) {

    const float spriteSize = 128;
    // move the sprite position
    float srcX = obj.currentAnimation != -1
                     ? obj.animations[obj.currentAnimation].currentFrame() * spriteSize
                     : 0.0f;

    SDL_FRect srcRect = {srcX, 0, spriteSize, spriteSize};
    SDL_FRect destRect = {obj.position.x, obj.position.y, spriteSize, spriteSize};

    SDL_RenderTextureRotated(
        state.renderer,
        obj.texture,
        &srcRect,
        &destRect,
        0,
        NULL,
        obj.direction == 1 ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
}

void update(
    const SDLState &state,
    GameState &gs,
    Resources &res,
    GameObject &obj,
    float deltaTime) {

    if (obj.type == ObjectType::PLAYER) {
        // input handling for the player
        // 0 means pressing neither A or D or BOTH
        float currentDirection = 0;

        // add one when A or D is pressed
        if (state.keys[SDL_SCANCODE_A]) {
            currentDirection -= 1;
        }

        if (state.keys[SDL_SCANCODE_D]) {
            currentDirection += 1;
        }

        // .. if both are pressed it becomes 0, and the character doesn't walk, because it
        // subtracts 1 (A-left) and then sums 1 (D-right)

        if (currentDirection != 0) {
            obj.direction = currentDirection;
        }

        PlayerState &playerState = obj.data.player.state;
        switch (playerState) {
        case PlayerState::IDLE: {
            if (currentDirection != 0) {
                playerState = PlayerState::WALKING;
                obj.texture = res.walkTexture;
                obj.currentAnimation = res.ANIM_PLAYER_WALK;
            } else {
                // when becomes idle it should decelerate, similar to breaking in cars
                if (obj.velocity.x) {

                    const float opposingFactor = obj.velocity.x > 0 ? -1.5f : 1.5f;
                    float opposingForce = opposingFactor * obj.acceleration.x * deltaTime;
                    if (std::abs(obj.velocity.x) < std::abs(opposingForce)) {
                        obj.velocity.x = 0;
                    } else {
                        // this applies an oposing force to the velocity, making it
                        // eventually reach 0.
                        //
                        // - if going right we're at a positive velocity.x the `amout`
                        // will then be a negative value that when added makes the
                        // velocity decrease until it reaches 0.
                        //
                        // - if going left velocity.x is negative. The `amout` will be
                        // POSITIVE that gets added to it, which eventually will make it
                        // become 0
                        obj.velocity.x += opposingForce;
                    }
                }
            }
            break;
        }
        case PlayerState::WALKING: {
            if (currentDirection == 0) {
                playerState = PlayerState::IDLE;
                obj.texture = res.idleTexture;
                obj.currentAnimation = res.ANIM_PLAYER_IDLE;
            }
            break;
        }
        case PlayerState::RUNNING: {
            std::cout << "RUNNING" << std::endl;

            break;
        }
        case PlayerState::JUMPING: {
            std::cout << "JUMPING" << std::endl;

            break;
        }
        }

        // accelerates the character
        obj.velocity += currentDirection * (obj.acceleration * deltaTime);

        if (std::abs(obj.velocity.x) > obj.maxSpeedX) {
            obj.velocity.x = currentDirection * obj.maxSpeedX;
        }

        // moves the character by velocity
        obj.position += obj.velocity * deltaTime;
    }
}
