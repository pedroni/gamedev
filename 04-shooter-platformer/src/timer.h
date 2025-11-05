#ifndef timer_h
#define timer_h

class Timer {
    float duration, time;

    bool timeout;

  public:
    Timer(float length) : duration(length), time(0) {}
    void step(float deltaTime) {
        time += deltaTime;

        // here we could've reset the time to 0, but we don't want that. we might lose a
        // few milliseconds or nanoseconds if we set to 0, this happens because our games
        // runs concurrently and a small fraction of time could've passed
        if (time >= duration) {
            timeout = true;
            time -= duration;
        }
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
