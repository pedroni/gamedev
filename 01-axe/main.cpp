#include "raylib.h"
#include <iostream>
#include <ostream>
#include <sys/syslimits.h>

int main() {
  const int screenWidth = 640;
  const int screenHeight = 360;
  InitWindow(screenWidth, screenHeight, "Axe Game");

  SetTargetFPS(60);

  int circleRadius = 25;
  int circleX = 0;
  int circleY = 0;
  int circleSpeed = 4;
  // circle edges
  int leftCircleX = circleX - circleRadius;
  int rightCircleX = circleX + circleRadius;
  int topCircleY = circleY - circleRadius;
  int bottomCircleY = circleY + circleRadius;

  // rectangle
  int rectSize = 80;

  int rectX = (screenWidth - rectSize) / 2;
  int rectY = 80;

  int rectDirection = 10;

  // rect edges
  int leftRectX = rectX;
  int rightRectX = rectX + rectSize;
  int topRectY = rectY;
  int bottomRectY = rectY + rectSize;

  bool collided = false;

  while (!WindowShouldClose()) {
    BeginDrawing();

    if (!collided) {

      // logic

      leftCircleX = circleX - circleRadius;
      rightCircleX = circleX + circleRadius;
      topCircleY = circleY - circleRadius;
      bottomCircleY = circleY + circleRadius;

      leftRectX = rectX;
      rightRectX = rectX + rectSize;
      topRectY = rectY;
      bottomRectY = rectY + rectSize;

      circleSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 8 : 4;

      if (IsKeyDown(KEY_A) && circleX > 0) {
        circleX -= circleSpeed * 1;
      }

      if (IsKeyDown(KEY_D) && circleX < screenWidth) {
        circleX += circleSpeed * 1;
      }

      if (IsKeyDown(KEY_W) && circleY > 0) {
        circleY -= circleSpeed * 1;
      }

      if (IsKeyDown(KEY_S) && circleY < screenHeight) {
        circleY += circleSpeed * 1;
      }

      // handle rect
      rectY += rectDirection;
      if (rectY > (screenHeight - rectSize) || rectY < 0) {
        rectDirection = -rectDirection;
      }

      collided = topCircleY <= bottomRectY && bottomCircleY >= topRectY &&
                 rightCircleX >= leftRectX && leftCircleX <= rightRectX;
    }

    ClearBackground(BLUE);
    DrawCircle(circleX, circleY, circleRadius, WHITE);
    DrawRectangle(rectX, rectY, rectSize, rectSize, BLACK);

    if (collided) {
      DrawText("GAME OVER", (screenWidth - 110) / 2, (screenHeight - 40) / 2,
               20, RED);
      if (IsKeyDown(KEY_SPACE)) {
        collided = false;
        circleX = 0;
        circleY = 0;
      }
    }

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
