#include "raylib.h"
#include <array>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <ostream>
#include <vector>

const int screenWidth = 800;
const int screenHeight = 450;
double elapsed = 0;

Texture2D textureFire;

bool isColliding(const Rectangle &A, const Rectangle &B) {
  bool xOverlap = A.x + A.width > B.x && B.x + B.width > A.x;
  bool yOverlap = A.y + A.height > B.y && B.y + B.height > A.y;

  if (xOverlap && yOverlap) {
    return true;
  }

  return false;
}

class Entity {
public:
  Entity() {}

  virtual void update(double dT) {}
  virtual void draw() {}
};

class Background : public Entity {

private:
  Texture2D texture_;
  float textureScale_{0.0};
  float scaledWidth_{0};
  int velocity_;

  Vector2 firstPos_{0, 0};
  Vector2 secondPos_{0, 0};

  float xPos_{0};

public:
  Background(Texture2D texture, int velocity) {
    texture_ = texture;

    textureScale_ = ((float)screenWidth / texture_.width);
    scaledWidth_ = texture_.width * textureScale_;

    velocity_ = velocity;
  }

  void update(double dT) override {
    xPos_ -= velocity_ * dT;

    if (xPos_ <= -scaledWidth_) {
      xPos_ = 0;
    }

    firstPos_.x = xPos_;
    secondPos_.x = xPos_ + scaledWidth_;
  }

  void draw() override {
    DrawTextureEx(texture_, firstPos_, 0.0, textureScale_, WHITE);
    DrawTextureEx(texture_, secondPos_, 0.0, textureScale_, WHITE);
  }
};

class Fireball : public Entity {

private:
  int spriteFrameCount_ = 5;
  Rectangle srcRect_;
  float initialPosY = 0;
  float initialPosX = 0;

public:
  int velocity = -500;
  bool colliding = false;
  Rectangle destRect;
  Fireball(int velocity, int posOffsetX, int posOffsetY) : velocity(velocity) {

    srcRect_.width = (float)textureFire.width / spriteFrameCount_;
    srcRect_.height = textureFire.height;
    srcRect_.x = 0;
    srcRect_.y = 0;

    initialPosX = screenWidth + posOffsetX;
    initialPosY = screenHeight - (40 + posOffsetY) * 2.5;

    destRect.x = initialPosX;
    destRect.y = initialPosY;
    destRect.width = srcRect_.width * 2.5;
    destRect.height = srcRect_.height * 2.5;
  }

  double collidingDuration = 0;
  void update(const double dT) override {
    if (colliding) {
      collidingDuration += dT;
    } else {
      collidingDuration = 0;
    }

    destRect.x += velocity * dT;
    if (destRect.x < -destRect.width) {
      destRect.x = initialPosX;
    }

    if (collidingDuration > 0.15) {
      destRect.x = initialPosX;
    }

    // 1/8 means 8 changes every second
    srcRect_.x = srcRect_.width *
                 (static_cast<int>(elapsed / (1.0 / 12)) % spriteFrameCount_);
  }

  void draw() override {
    DrawTexturePro(textureFire, srcRect_, destRect, Vector2{0, 0}, 0,
                   colliding ? BLACK : WHITE);
  }
};

int main() {
  std::srand(std::time(nullptr));
  std::vector<Entity *> entities;

  InitWindow(screenWidth, screenHeight, "Dapper Dasher");
  InitAudioDevice();

  Sound backgroundSound = LoadSound("./assets/ridiculousgravewalk.ogg");
  Sound jumpLandingSound = LoadSound("./assets/jump_landing.mp3");
  SetSoundVolume(backgroundSound, 0.8);
  PlaySound(backgroundSound);

  Music grunts = LoadMusicStream("./assets/grunts.wav");

  std::array<float, 2> deathGrunt = {0.5, 1.8};

  const int HURT_GRUNT_SIZE = 2;
  std::array<std::array<float, 2>, HURT_GRUNT_SIZE> hurtGrunts;
  hurtGrunts[0] = {39, 0.5};
  hurtGrunts[1] = {7, 0.5};

  std::array<float, 2> playingGrunt = hurtGrunts[0];

  float gruntPlayedAt = elapsed;

  // Move the window to the right side of the monitor
  int monitor = GetCurrentMonitor();
  int monitorWidth = GetMonitorWidth(monitor);
  int monitorHeight = GetMonitorHeight(monitor);

  SetWindowPosition(monitorWidth - screenWidth,
                    (monitorHeight - screenHeight) / 2);

  textureFire = LoadTexture("./assets/fireball-sheet.png");

  Texture2D textureSky = LoadTexture("./assets/background/sky.png");

  // 800 / 1920 = ~0.41, before i had the value hardcoded but we can make it
  // dynamic since we know the screen width and the texture width
  const float skyScale = (float)screenWidth / textureSky.width;

  Texture2D textureGraves = LoadTexture("./assets/background/graves.png");

  Background bgSky{textureSky, 20};
  Background gravesBg{textureGraves, 30};
  Background backTreesBg{LoadTexture("./assets/background/back_trees.png"), 40};
  Background cryptBg{LoadTexture("./assets/background/crypt.png"), 45};
  Background wallBg{LoadTexture("./assets/background/wall.png"), 50};
  Background groundBg{LoadTexture("./assets/background/ground.png"), 100};
  Background treeBg{LoadTexture("./assets/background/tree.png"), 100};
  Background bonesBg{LoadTexture("./assets/background/bones.png"), 100};
  entities.push_back(&bgSky);
  entities.push_back(&gravesBg);
  entities.push_back(&backTreesBg);
  entities.push_back(&cryptBg);
  entities.push_back(&wallBg);
  entities.push_back(&groundBg);
  entities.push_back(&treeBg);
  entities.push_back(&bonesBg);

  // acceleration due to gravity (pixel/second)/second
  const int gravity = 3400;

  double velocityY = 0;

  // velocity pixel/second

  Texture2D textureWalk = LoadTexture("./assets/hero/Walk.png");
  int textureWalkSprites = 8;

  Texture2D textureRun = LoadTexture("./assets/hero/Run.png");
  int textureRunSprites = 7;

  Texture2D textureDead = LoadTexture("./assets/hero/Dead.png");
  int textureDeadSprites = 6;

  Texture2D textureJump = LoadTexture("./assets/hero/Jump.png");
  int textureJumpSprites = 6;

  Texture2D textureHurt = LoadTexture("./assets/hero/Hurt.png");
  int textureHurtSprites = 2;

  Rectangle heroRect;
  heroRect.x = 0;
  heroRect.y = 0;
  heroRect.width = static_cast<float>(textureWalk.width) / textureWalkSprites;
  heroRect.height = textureWalk.height;
  const float heroScale = 2.5;
  const int jumpingVelocity = -400 * heroScale;

  // renders the hero in in the middle of the screen
  Rectangle heroPos;
  heroPos.x = 120;
  heroPos.y = screenHeight - heroRect.height * heroScale - 40;
  heroPos.width = heroRect.width * heroScale;
  heroPos.height = heroRect.height * heroScale;

  Rectangle heroCollidingRect{heroPos.x + 50, heroPos.y + heroPos.width,
                              heroPos.width / 5, heroPos.height / 2};

  bool dead = false;
  int colliding = 0;
  Rectangle heroRectHealth{20, 20, 200, 5};

  const float groundPos = heroPos.y;

  Fireball fireOne{-600, 400, 0};
  Fireball fireTwo{-600, 1000, 0};
  entities.push_back(&fireOne);
  entities.push_back(&fireTwo);

  Texture2D *heroTexture;

  SetTargetFPS(60);
  // Main game loop
  while (!WindowShouldClose()) {
    // time since last frame
    double dT = GetFrameTime();
    elapsed += dT;

    UpdateMusicStream(grunts);

    BeginDrawing();

    ClearBackground(RAYWHITE);

    bool onTheGround = heroPos.y >= groundPos;

    if (onTheGround) {
      // if the character is on the ground it should have no velocity, never
      heroPos.y = groundPos;
      velocityY = 0;
    } else {
      // in the air
      //
      // velocity is "decreased" because of gravity every frame the character is
      // in the air
      //
      // Y = 0 (character is at the top, and will fall down)
      // imagine the velocity is 10, on every loop when jumping it will y will
      // change the velocity velocity changes position, gravity changes
      // velocity. first iteration velocity = 10 Y = 10 velocity = 11 Y = 22
      //
      // velocity = 12
      // Y = 22+12=34
      //
      // velocity = 13
      // Y = 34+13=47
      //
      // ... and so on. Gravity is 1 and it increases velocity on every
      // iteration whenever the character is on the air
      velocityY += gravity * dT;

      // play jumping sound, there's some seconds in the beggining of the audio
      // so we can just straight play it, when the characters falls it plays at
      // the exact time.
      PlaySound(jumpLandingSound);
    }

    if (!dead) {
      // can only jump when on the ground
      if (onTheGround && IsKeyDown(KEY_SPACE)) {
        velocityY += jumpingVelocity;
        std::cout << "triggered jump" << std::endl;
      }

      // resets animation so that jump can start
      heroRect.x = 0;
    }

    // update animation frame of hero
    if (onTheGround) {
      if (dead) {
        // if statement makes it stop changing the x after it reached the last
        // sprite from the spritesheet
        if (heroRect.x < heroRect.width * (textureDeadSprites - 1)) {
          heroRect.x = heroRect.width * (static_cast<int>(elapsed / (1.0 / 4)) %
                                         textureDeadSprites);
        }
      } else {
        heroRect.x = heroRect.width * (static_cast<int>(elapsed / (1.0 / 4)) %
                                       textureRunSprites);
      }
    } else {
      // jumping sprite
      heroRect.x = heroRect.width *
                   (static_cast<int>(elapsed / (1.0 / 5)) % textureJumpSprites);
    }
    // changes the position based off the velocity, velocity is hwo much we
    // change the position in time by frames
    heroPos.y += velocityY * dT;

    // end update animation frame of hero
    for (auto &entity : entities) {

      Fireball *fireball = dynamic_cast<Fireball *>(entity);

      if (!dead) {
        if (fireball != nullptr) {
          if (isColliding(fireball->destRect, heroCollidingRect)) {
            if (!fireball->colliding) {
              // do the damager when first collided, which means the flag wasnt
              // set to true yet. once it collided it's set to true, then it
              // goes back to false when it stops colliding
              heroRectHealth.width -= 50;

              // find a random grunt each time
              playingGrunt = hurtGrunts[std::rand() % (HURT_GRUNT_SIZE)];
              gruntPlayedAt = elapsed;
              SeekMusicStream(grunts, playingGrunt[0]);
              PlayMusicStream(grunts);
              colliding++;
            }
            fireball->colliding = true;
          } else {
            if (gruntPlayedAt > 0 &&
                elapsed - gruntPlayedAt >= playingGrunt[1]) {
              // if grunt ever played when it finishes colliding we need to stop
              // the music
              StopMusicStream(grunts);
              gruntPlayedAt = 0;
              colliding--;
            }
            fireball->colliding = false;
          }
        }

        entity->update(dT);
      } else {
        if (fireball != nullptr) {
          fireball->velocity = 0;
          fireball->destRect.y = -80;
        }
      }

      entity->draw();
    }

    heroCollidingRect.y = heroPos.y + heroPos.height / 2;

    if (dead) {
      heroTexture = &textureDead;
    } else if (colliding > 0) {
      heroTexture = &textureHurt;
    } else if (!onTheGround) {
      heroTexture = &textureJump;
    } else {
      heroTexture = &textureWalk;
    }

    DrawTexturePro(*heroTexture, heroRect, heroPos, Vector2{0, 0}, 0.0, WHITE);
    // DrawRectangleLines(heroCollidingRect.x, heroCollidingRect.y,
    //                    heroCollidingRect.width, heroCollidingRect.height,
    //                    RED);

    // draw health bar
    DrawRectangle(heroRectHealth.x, heroRectHealth.y, heroRectHealth.width,
                  heroRectHealth.height, RED);
    DrawRectangle(heroRectHealth.x, heroRectHealth.y + heroRectHealth.height,
                  heroRectHealth.width, heroRectHealth.height, MAROON);

    if (heroRectHealth.width <= 0) {
      if (!dead) {
        playingGrunt = deathGrunt;
        SeekMusicStream(grunts, playingGrunt[0]);
        PlayMusicStream(grunts);
        gruntPlayedAt = elapsed;

        // dead sprite reset to 0
        heroRect.x = 0;

        dead = true;
      }

      // after certain time passed after grunt play sound
      if (gruntPlayedAt > 0 && elapsed - gruntPlayedAt >= playingGrunt[1]) {
        StopMusicStream(grunts);
        gruntPlayedAt = 0;
      }
      DrawRectangle((screenWidth / 2) - 146, (screenHeight / 2) - 10, 280, 56,
                    Color{0, 0, 0, 200});
      DrawText("GAME OVER", (screenWidth / 2) - 128, screenHeight / 2, 40,
               MAROON);
    }

    EndDrawing();
  }

  // De-Initialization
  UnloadTexture(textureWalk);
  UnloadTexture(textureJump);
  UnloadTexture(textureRun);
  UnloadTexture(textureFire);
  UnloadTexture(textureSky);
  CloseWindow();

  return 0;
}
