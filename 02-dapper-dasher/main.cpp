#include "raylib.h"

int main() {
  const int screenWidth = 800;
  const int screenHeight = 450;
  int price;
  price = 20;
  price = 25;
  InitWindow(screenWidth, screenHeight, "01-axe Game");

  SetTargetFPS(60);

  // Main game loop
  while (!WindowShouldClose()) {
    // Update

    // Draw
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Congrats! You created your first raylib window!", 190, 200, 20,
             LIGHTGRAY);
    EndDrawing();
  }

  // De-Initialization
  CloseWindow();

  return 0;
}
