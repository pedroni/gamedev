#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_scancode.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_timer.h"
#include "SDL3/SDL_video.h"
#include "animation.h"
#include "gameobject.h"
#include "state.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3_image/SDL_image.h>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <ios>
#include <iostream>
#include <vector>

const char *formatText(const char *fmt, ...) {
    static char buffer[256]; // static = persists after function returns
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    return buffer;
}

const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_CHARACTERS = 1;
const int MAX_LAYERS = 2;
const int MAP_ROWS = 5;
const int MAP_COLS = 50;
const int TILE_SIZE = 32;

struct GameState {
    std::array<std::vector<GameObject>, MAX_LAYERS> layers;

    // so we know where the placer is at
    int playerIndex;

    GameState() {
        playerIndex = -1; // will change automatically on map loading
    }

    GameObject &player() { return layers[LAYER_IDX_CHARACTERS][playerIndex]; };
};

struct Resources {
    const int ANIM_PLAYER_IDLE = 0;
    const int ANIM_PLAYER_WALK = 1;
    const int ANIM_PLAYER_RUN = 2;

    std::vector<Animation> playerAnims;
    std::vector<SDL_Texture *> textures;
    SDL_Texture *idleTexture, *runTexture, *walkTexture, *grassTexture, *groundTexture,
        *panelTexture, *brickTexture;

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

        // player
        idleTexture = loadTexture(state.renderer, "./assets/light/Idle.png");
        runTexture = loadTexture(state.renderer, "./assets/light/Run.png");
        walkTexture = loadTexture(state.renderer, "./assets/light/Walk.png");

        // map
        grassTexture = loadTexture(state.renderer, "./assets/map/grass.png");
        groundTexture = loadTexture(state.renderer, "./assets/map/ground.png");
        panelTexture = loadTexture(state.renderer, "./assets/map/panel.png");
        brickTexture = loadTexture(state.renderer, "./assets/map/brick.png");
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
GameObject createObject(const SDLState &state, int r, int c, ObjectType type);
void createTiles(const SDLState &state, GameState &gs, Resources &res);
void checkCollision(
    const SDLState &state,
    GameState &gs,
    Resources &res,
    GameObject &objA,
    GameObject &objB,
    float deltaTime);
void handleKeyInput(
    const SDLState &state,
    GameState &gs,
    GameObject &obj,
    SDL_Scancode key,
    bool keyDown);

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
    createTiles(state, gs, res);

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
            case SDL_EVENT_WINDOW_RESIZED: {
                state.width = event.window.data1;
                state.height = event.window.data2;
                break;
            }
            case SDL_EVENT_KEY_DOWN: {
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }
                handleKeyInput(state, gs, gs.player(), event.key.scancode, true);
                break;
            }
            case SDL_EVENT_KEY_UP: {
                handleKeyInput(state, gs, gs.player(), event.key.scancode, false);
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

        // display some debug info
        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(
            state.renderer,
            8,
            8,
            formatText("State: %d", gs.player().data.player.state));

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

    state.width = 640 * 2;
    state.height = 320 * 2;
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

    const float spriteSize = obj.type == ObjectType::PLAYER ? 128 : TILE_SIZE;

    // move the sprite position
    float srcX = obj.currentAnimation != -1
                     ? obj.animations[obj.currentAnimation].currentFrame() * spriteSize
                     : 0.0f;

    SDL_FRect srcRect = {srcX, 0, spriteSize, spriteSize};
    SDL_FRect destRect = {obj.position.x, obj.position.y, TILE_SIZE, TILE_SIZE};

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

    // apply gravity to dynamic objects
    if (obj.dynamic) {
        obj.velocity +=
            glm::vec2(0, 200.0f) * deltaTime; // apply downward force to objects
    }

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
        }

        // accelerates the character by pressing keys
        obj.velocity += currentDirection * (obj.acceleration * deltaTime);

        if (std::abs(obj.velocity.x) > obj.maxSpeedX) {
            obj.velocity.x = currentDirection * obj.maxSpeedX;
        }
    }

    // moves the object by velocity overtime
    obj.position += obj.velocity * deltaTime;

    // handle collision detection
    // ⚠️ THIS IS BAD OPTMIZED THERE'S A BETTER WAY TO DO IT
    // the current "obj" in the update game loop is our objA, whereas the objects in the
    // layers are our objB
    bool foundGround = false;
    for (std::vector<GameObject> &layer : gs.layers) {
        for (GameObject &objB : layer) {
            // make sure they're different by checking their memory address
            // we don't want to check if it's colliding against itself
            if (&obj != &objB) {
                checkCollision(state, gs, res, obj, objB, deltaTime);

                // grounded sensor, this creates a pixel line that is at the bottom of the
                // current object collider
                SDL_FRect sensor = {
                    obj.position.x + obj.collider.x,
                    obj.position.y + obj.collider.y + obj.collider.h,
                    obj.collider.w,
                    1};

                SDL_FRect rectB = {
                    objB.position.x + objB.collider.x,
                    objB.position.y + objB.collider.y,
                    objB.collider.w,
                    objB.collider.h};

                if (SDL_HasRectIntersectionFloat(&sensor, &rectB)) {
                    foundGround = true;
                }
            }
        }
    }
    // if they're different it means that we're changing state
    if (obj.grounded != foundGround) {
        obj.grounded = foundGround;
        if (foundGround && obj.type == ObjectType::PLAYER) {
            obj.data.player.state = PlayerState::WALKING;
        }
    }
}

GameObject createObject(
    const SDLState &state,
    int row,
    int col,
    ObjectType type,
    SDL_Texture *texture) {
    GameObject obj;
    obj.type = type;
    obj.texture = texture;
    obj.position = glm::vec2(col * TILE_SIZE, state.logH - (MAP_ROWS - row) * TILE_SIZE);
    obj.collider = {0, 0, TILE_SIZE, TILE_SIZE};

    return obj;
};

void collisionResponse(
    const SDLState &state,
    GameState &gs,
    Resources &res,
    GameObject &objA,
    GameObject &objB,
    SDL_FRect &rectA,
    SDL_FRect &rectB,
    SDL_FRect &rectC,
    float deltaTime) {
    // object we're checking
    if (objA.type == ObjectType::PLAYER) {

        // object it is colliding with
        switch (objB.type) {
        case ObjectType::LEVEL: {
            if (rectC.w < rectC.h) {
                // if height is bigger than width
                // horizontal collision, when walking X

                if (objA.velocity.x > 0) { // going right, positive velocity
                    // "teleport" the character back the size of collision
                    objA.position.x -= rectC.w;

                } else if (objA.velocity.x < 0) { // negative velocity x, means going left
                    // should be less than 0, because we don't want to do anything if its
                    // 0 "teleport" the character back the size of collision
                    objA.position.x += rectC.w;
                }

                objA.velocity.x = 0; // prevent the player from moving
            } else {
                // vertical collision, when falling/jumping Y
                if (objA.velocity.y > 0) { // going up, positive velocity
                    // "teleport" the character back the size of collision
                    objA.position.y -= rectC.h;
                } else if (objA.velocity.y < 0) { // negative velocity y, means going
                                                  // down/falling gravity
                    // "teleport" the character back the size of collision
                    objA.position.y += rectC.h;
                }
                objA.velocity.y = 0; // prevent the player from moving
            }
        }
        }
    }
}

void checkCollision(
    const SDLState &state,
    GameState &gs,
    Resources &res,
    GameObject &objA,
    GameObject &objB,
    float deltaTime) {
    // rects here are the hitbox

    SDL_FRect rectA = {
        objA.position.x + objA.collider.x,
        objA.position.y + objA.collider.y,
        objA.collider.w,
        objA.collider.h};

    SDL_FRect rectB = {
        objB.position.x + objB.collider.x,
        objB.position.y + objB.collider.y,
        objB.collider.w,
        objB.collider.h};

    // the intersection object generated when A and B are intersecting
    SDL_FRect rectC = {0, 0, 0, 0};

    if (SDL_GetRectIntersectionFloat(&rectA, &rectB, &rectC)) {
        // found intersection, it means that they're colliding
        collisionResponse(state, gs, res, objA, objB, rectA, rectB, rectC, deltaTime);
    }
};

void createTiles(const SDLState &state, GameState &gs, Resources &res) {
    /**
     * 0 - Empty tiles
     * 1 - Ground
     * 2 - Panel
     * 3 - Enemy
     * 4 - Player
     * 5 - Grass
     * 6 - Brick
     */
    short map[MAP_ROWS][MAP_COLS] = {
        // clang-format off
        {0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        // clang-format on
    };

    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLS; col++) {
            switch (map[row][col]) {
            case 1: // ground
            {
                GameObject obj =
                    createObject(state, row, col, ObjectType::LEVEL, res.groundTexture);
                gs.layers[LAYER_IDX_LEVEL].push_back(obj);
                break;
            }
            case 2: // ground
            {
                GameObject obj =
                    createObject(state, row, col, ObjectType::LEVEL, res.panelTexture);

                gs.layers[LAYER_IDX_LEVEL].push_back(obj);
                break;
            }
            case 4: // player
            {
                // create the player
                GameObject player =
                    createObject(state, row, col, ObjectType::PLAYER, res.idleTexture);
                player.data.player = PlayerData();
                player.animations = res.playerAnims;
                player.currentAnimation = res.ANIM_PLAYER_IDLE;
                // when pressing the "acelerador" do carro ele acelera 300
                player.acceleration = glm::vec2(300, 0);
                player.maxSpeedX = 100;

                // define gravity
                // ⚠️ should i call "hasGravity"
                player.dynamic = true;
                // ⚠️ not the perfect hit box our sprite character is bigger

                player.collider = {10, 10, 12, 26};

                gs.layers[LAYER_IDX_CHARACTERS].push_back(player);
                gs.playerIndex = gs.layers[LAYER_IDX_CHARACTERS].size() - 1;
                break;
            }
            }
        }
        // we always need to set the player in order for the game to run
        assert(gs.playerIndex != -1);
    }
}

void handleKeyInput(
    const SDLState &state,
    GameState &gs,
    GameObject &obj,
    SDL_Scancode key,
    bool keyDown) {

    if (obj.type != ObjectType::PLAYER) {
        return;
    }

    const float JUMP_FORCE = -200.0f;

    switch (obj.data.player.state) {
    case PlayerState::IDLE: {
        if (key == SDL_SCANCODE_SPACE && keyDown) {
            obj.data.player.state = PlayerState::JUMPING;
            obj.velocity.y += JUMP_FORCE;
        }
        break;
    }
    case PlayerState::WALKING: {
        if (key == SDL_SCANCODE_SPACE && keyDown) {
            obj.data.player.state = PlayerState::JUMPING;
            obj.velocity.y += JUMP_FORCE;
        }
        break;
    }
    }
}
