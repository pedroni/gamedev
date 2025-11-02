#ifndef animation_h
#define animation_h

#include "timer.h"

class Animation {
    Timer timer;
    int frameCount;

  public:
    Animation() : timer(Timer(0)), frameCount(0) {}
    Animation(int frameCount, float duration) : timer(duration), frameCount(frameCount) {}

    float getLength() const { return timer.getDuration(); }

    int currentFrame() const {
        return static_cast<int>(timer.getTime() / timer.getDuration() * frameCount);
    }
    void step(float deltaTime) { timer.step(deltaTime); }
};

#endif
