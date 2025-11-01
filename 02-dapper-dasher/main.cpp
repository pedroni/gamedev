#include "raylib.h"
#include <iostream>
#include <vector>

const int screenWidth = 800;
const int screenHeight = 450;
double elapsed = 0;

Texture2D textureFire;

class Fireball {

private:
  int velocity_ = -500;
  int spriteFrameCount_ = 5;
  Rectangle rect_;
  Vector2 pos_;

public:
  Fireball(int velocity, int posOffsetX, int posOffsetY) : velocity_(velocity) {

    rect_.width = (float)textureFire.width / spriteFrameCount_;
    rect_.height = textureFire.height;
    rect_.x = 0;
    rect_.y = 0;
    pos_.x = screenWidth + posOffsetX;
    pos_.y = screenHeight - (30 + posOffsetY);
  }

  void update(const double dT) {
    pos_.x += velocity_ * dT;
    if (pos_.x < -rect_.width) {
      pos_.x = screenWidth;
    }

    // 1/8 means 8 changes every second
    rect_.x = rect_.width *
              (static_cast<int>(elapsed / (1.0 / 8)) % spriteFrameCount_);
  }

  void draw() { DrawTextureRec(textureFire, rect_, pos_, WHITE); }
};

int main() {

  InitWindow(screenWidth, screenHeight, "Dapper Dasher");

  // Move the window to the right side of the monitor
  int monitor = GetCurrentMonitor();
  int monitorWidth = GetMonitorWidth(monitor);
  int monitorHeight = GetMonitorHeight(monitor);

  SetWindowPosition(monitorWidth - screenWidth,
                    (monitorHeight - screenHeight) / 2);

  textureFire = LoadTexture("./assets/fireball-sheet.png");

  // acceleration due to gravity (pixel/second)/second
  const int gravity = 3000;

  double velocityY = 0;

  // velocity pixel/second
  const int jumpingVelocity = -800;

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

  // renders the hero in in the middle of the screen
  Vector2 heroPos;
  heroPos.x = (static_cast<float>(screenWidth) / 2) - (heroRect.width / 2);
  heroPos.y = screenHeight - heroRect.height - 10;

  const float groundPos = heroPos.y;

  std::vector<Fireball> fireballs;
  fireballs.push_back(Fireball(-300, 0, 0));
  fireballs.push_back(Fireball(-300, 200, 0));
  fireballs.push_back(Fireball(-400, 500, 30));

  SetTargetFPS(60);
  // Main game loop
  while (!WindowShouldClose()) {
    // time since last frame
    double dT = GetFrameTime();
    elapsed += dT;

    BeginDrawing();

    ClearBackground(RAYWHITE);

    // std::cout << posY << std::endl;

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
                   (static_cast<int>(elapsed / (1.0 / 8)) % textureRunSprites);
    }
    // end update animation frame of hero

    DrawTextureRec(textureWalk, heroRect, heroPos, WHITE);

    for (auto &fireball : fireballs) {
      fireball.update(dT);
      fireball.draw();
    }

    EndDrawing();
  }

  // De-Initialization
  UnloadTexture(textureWalk);
  UnloadTexture(textureJump);
  UnloadTexture(textureRun);
  CloseWindow();

  return 0;
}
