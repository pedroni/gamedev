#ifndef gameobject_h
#define gameobject_h

#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "animation.h"
#include <glm/glm.hpp>
#include <vector>

enum class PlayerState { IDLE, WALKING, RUNNING, JUMPING };

struct PlayerData {
    PlayerState state;
    PlayerData() { state = PlayerState::IDLE; }
};

struct LevelData {};
struct EnemyData {};

union ObjectData {
    PlayerData player;
    EnemyData enemy;
    LevelData level;

    ObjectData() { player = PlayerData(); }
};

enum class ObjectType { PLAYER, LEVEL, ENEMY };

// every single object in the game
struct GameObject {
    ObjectType type;
    ObjectData data;

    glm::vec2 position, velocity, acceleration;

    // 1 right, -1 left
    float direction;
    float maxSpeedX;

    std::vector<Animation> animations;
    int currentAnimation;

    SDL_Texture *texture;

    /**
     * controls whether its affected by gravity or not
     */
    bool dynamic;

    // custom hitbox for our game object
    SDL_FRect collider;

    GameObject() {
        data = ObjectData();
        type = ObjectType::LEVEL;

        direction = 1;
        // max speed is used here to make sure we dont have infinite acceleration
        maxSpeedX = 0;

        position = velocity = acceleration = glm::vec2(0);

        // when -1 it's unset
        currentAnimation = -1;

        // by default objects don't have gravity
        dynamic = false;

        // by default objects aren't "collideable"
        collider = {0, 0, 0, 0};

        texture = NULL;
    }
};

#endif
