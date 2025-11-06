#ifndef timer_h
#define timer_h

class Timer {
    float duration, time;

    bool timeout;

  public:
    Timer(float length) : duration(length), time(0) {}
    bool step(float deltaTime) {
        time += deltaTime;

        // here we could've reset the time to 0, but we don't want that. we might lose a
        // few milliseconds or nanoseconds if we set to 0, this happens because our games
        // runs concurrently and a small fraction of time could've passed
        if (time >= duration) {
            // once it has finished for the first time we set to true, note that we still
            // keep adding time, because we let the consumer handle what it want to do
            // with timeout, on example is the colliding bullet, where we set to inactive
            // once the first "iteration/loop" of the sprite sheet has finished
            timeout = true;
            time -= duration;
            return true;
        }
        return false;
    }

    bool isTimeout() const { return timeout; }
    float getTime() const { return time; }
    float getDuration() const { return duration; };
    void reset() {
        time = 0;
        timeout = false;
    }
};

#endif
