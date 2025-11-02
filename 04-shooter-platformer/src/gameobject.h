#ifndef gameobject_h
#define gameobject_h

#include "SDL3/SDL_render.h"
#include "animation.h"
#include <glm/glm.hpp>
#include <vector>

enum class ObjectType { player, level, enemy };

// every single object in the game
struct GameObject {
    ObjectType type;
    glm::vec2 position, velocity, acceleration;

    // 1 right, -1 left
    float direction;

    std::vector<Animation> animations;
    int currentAnimation;

    SDL_Texture *texture;

    GameObject() {
        type = ObjectType::level;

        direction = 1;
        position = velocity = acceleration = glm::vec2(0);

        // when -1 it's unset
        currentAnimation = -1;

        texture = NULL;
    }
};

#endif
