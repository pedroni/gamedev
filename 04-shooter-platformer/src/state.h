#ifndef state_h
#define state_h

#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"

struct SDLState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    // logW and logH means logical width/height it's used by
    // SDL_SetRenderLogicalPresentation to make a logical width of our game without us
    // having to worry about window resize and screen resolution, width and height here
    // are the actual widnow size on the client
    int width, height, logW, logH;

    const bool *keys;
    SDLState() { keys = SDL_GetKeyboardState(NULL); }
};

#endif
