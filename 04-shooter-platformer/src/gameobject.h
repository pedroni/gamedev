#ifndef gameobject_h
#define gameobject_h

#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "animation.h"
#include "timer.h"
#include <glm/glm.hpp>
#include <vector>

enum class PlayerState { IDLE, WALKING, RUNNING, JUMPING };
enum class BulletState { MOVING, COLLIDING, INACTIVE };
enum class EnemyState { IDLE, WALKING, DAMAGED, DEAD };

struct PlayerData {
    PlayerState state;
    Timer weaponTimer;

    // weapon timer here is working like a COOLDOWN to a specific skill! that's fucking
    // cool. I imagine a skill system where we'd have a specific timer for each skill in
    // the game. so we'd have a SkillObject or something like that would have their
    // specific timers for when they're cast
    PlayerData() : state(PlayerState::IDLE), weaponTimer(0.8f) {}
};

struct LevelData {};

struct EnemyData {
    EnemyState state;
    Timer damagedTimer;
    int health;
    EnemyData() : state(EnemyState::IDLE), damagedTimer(0.5f), health(20) {}
};

struct BulletData {
    BulletState state;
    // ⚠️ every object has this data
    BulletData() : state(BulletState::MOVING) {}
};

// 🚨 in the original course its a union but for c++98 / c++11 we can't do that, we do
// have to manager pointers for simplicity i made this a struct, though it's badly
// optimized because now all objects in our game contains all this data stored in memory,
// even though we're not using it
struct ObjectData {
    PlayerData player;
    EnemyData enemy;
    LevelData level;
    BulletData bullet;
};

enum class ObjectType { PLAYER, LEVEL, ENEMY, BULLET };

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

    bool grounded;

    Timer flashTimer;
    bool shouldFlash;

    int animationFrame;

    GameObject() : flashTimer(0.05f) {
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

        grounded = false;

        texture = NULL;

        shouldFlash = false;

        animationFrame = 1;
    }
};

#endif
