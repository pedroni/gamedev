#include "raylib.h"
#include <iostream>
#include <vector>

const int screenWidth = 800;
const int screenHeight = 450;
double elapsed = 0;

Texture2D textureFire;

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
    std::cout << "####the scaled width" << scaledWidth_ << std::endl;

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
  int velocity_ = -500;
  int spriteFrameCount_ = 5;
  Rectangle srcRect_;
  Rectangle destRect_;
  float initialPosY = 0;
  float initialPosX = 0;

public:
  Fireball(int velocity, int posOffsetX, int posOffsetY) : velocity_(velocity) {

    srcRect_.width = (float)textureFire.width / spriteFrameCount_;
    srcRect_.height = textureFire.height;
    srcRect_.x = 0;
    srcRect_.y = 0;

    initialPosX = screenWidth + posOffsetX;
    initialPosY = screenHeight - (40 + posOffsetY) * 2.5;

    destRect_.x = initialPosX;
    destRect_.y = initialPosY;
    destRect_.width = srcRect_.width * 2.5;
    destRect_.height = srcRect_.height * 2.5;
  }

  void update(const double dT) override {
    destRect_.x += velocity_ * dT;
    if (destRect_.x < -destRect_.width) {
      destRect_.x = initialPosX;
    }

    // 1/8 means 8 changes every second
    srcRect_.x = srcRect_.width *
                 (static_cast<int>(elapsed / (1.0 / 12)) % spriteFrameCount_);
  }

  void draw() override {
    DrawTexturePro(textureFire, srcRect_, destRect_, Vector2{0, 0}, 0, WHITE);
  }
};

int main() {

  std::vector<Entity *> entities;

  InitWindow(screenWidth, screenHeight, "Dapper Dasher");

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

  Texture2D textureJump = LoadTexture("./assets/hero/Jump.png");

  Rectangle heroRect;
  heroRect.x = 0;
  heroRect.y = 0;
  heroRect.width = static_cast<float>(textureWalk.width) / textureWalkSprites;
  heroRect.height = textureWalk.height;
  const float heroScale = 2.5;
  const int jumpingVelocity = -400 * heroScale;

  // renders the hero in in the middle of the screen
  Vector2 heroPos;
  heroPos.x = (static_cast<float>(screenWidth) / 2) - (heroRect.width / 2);
  heroPos.y = screenHeight - heroRect.height * heroScale - 40;

  const float groundPos = heroPos.y;

  Fireball fireOne{-600, 300, 0};
  Fireball fireTwo{-600, 600, 0};
  entities.push_back(&fireOne);
  entities.push_back(&fireTwo);

  SetTargetFPS(60);
  // Main game loop
  while (!WindowShouldClose()) {
    // time since last frame
    double dT = GetFrameTime();
    elapsed += dT;

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
    }

    // can only jump when on the ground
    if (onTheGround && IsKeyDown(KEY_SPACE)) {
      velocityY += jumpingVelocity;
      std::cout << "triggered jump" << std::endl;
    }

    // changes the position based off the velocity, velocity is hwo much we
    // change the position in time by frames
    heroPos.y += velocityY * dT;

    // update animation frame of hero
    if (onTheGround) {
      heroRect.x = heroRect.width *
                   (static_cast<int>(elapsed / (1.0 / 4)) % textureRunSprites);
    }
    // end update animation frame of hero

    for (auto &entity : entities) {
      entity->update(dT);
      entity->draw();
    }

    DrawTexturePro(textureWalk, heroRect,
                   Rectangle{heroPos.x, heroPos.y, heroRect.width * heroScale,
                             heroRect.height * heroScale},
                   Vector2{0, 0}, 0.0, WHITE);

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
