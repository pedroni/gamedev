#include "raylib.h"
#include <cmath>
#include <iostream>

int main() {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "Dapper Dasher");

  // Move the window to the right side of the monitor
  int monitor = GetCurrentMonitor();
  int monitorWidth = GetMonitorWidth(monitor);
  int monitorHeight = GetMonitorHeight(monitor);

  SetWindowPosition(monitorWidth - screenWidth,
                    (monitorHeight - screenHeight) / 2);

  SetTargetFPS(60);

  int rectWidth = 20;
  int rectHeight = 40;

  // 450 - 40 = 410
  const int initialPosY = screenHeight - rectHeight;
  // 450 - 80 = 370
  const int maxPosY = initialPosY - rectHeight * 2;

  int posY = initialPosY;
  double velocityY = 0;

  bool jumping = false;

  const int jumpingVelocity = 5;

  // Main game loop
  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(RAYWHITE);

    std::cout << posY << std::endl;

    if (IsKeyDown(KEY_SPACE) && !jumping) {
      velocityY = -jumpingVelocity;
      jumping = true;
      std::cout << "triggered jump" << std::endl;
    }

    // here reached the max height of the jump in the y direction, now we need
    // to go down
    if (posY <= maxPosY) {
      velocityY = jumpingVelocity;
    }

    // changes the position based off the velocity, velocity is hwo much we
    // change the position in time by frames
    posY += velocityY;

    // prevent the character from moving out of the screen when going down on
    // the jump animation direction
    if (posY >= initialPosY) {
      jumping = false;
      velocityY = 0;
    }

    DrawRectangle(10, posY, rectWidth, rectHeight, RED);

    EndDrawing();
  }

  // De-Initialization
  CloseWindow();

  return 0;
}
