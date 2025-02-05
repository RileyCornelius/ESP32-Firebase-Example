#pragma once

#include <Arduino.h>

/**--------------------------------------------------------------------------------------
 * Millis Timer Class
 *-------------------------------------------------------------------------------------*/

class SimpleTimer
{
protected:
    uint32_t lastTrigger;
    uint32_t period;

public:
    SimpleTimer()
    {
        reset();
        period = 1;
    }
    SimpleTimer(uint32_t period)
    {
        reset();
        setPeriod(period);
    }

    virtual uint32_t getTime() { return millis(); }
    uint32_t getPeriod() { return period; }
    uint32_t getElapsed() { return getTime() - lastTrigger; }
    uint32_t getRemaining() { return period - getElapsed(); }
    void setPeriod(uint32_t period) { this->period = period; }
    void reset() { lastTrigger = getTime(); }
    bool ready()
    {
        bool isReady = (getElapsed() >= period);
        if (isReady)
        {
            reset();
        }
        return isReady;
    }

    operator bool() { return ready(); }
};

/**--------------------------------------------------------------------------------------
 * Micros Timer Class
 *-------------------------------------------------------------------------------------*/

class TimerMicros : public SimpleTimer
{
    using SimpleTimer::SimpleTimer; // inherit constructors
public:
    uint32_t getTime() override { return micros(); };
};

/**--------------------------------------------------------------------------------------
 * Timer Macros
 *-------------------------------------------------------------------------------------*/

#define MILLIS_TO_SECONDS(n) (n / 1000)
#define MILLIS_TO_MINUTES(n) (n / 60000)
#define MILLIS_TO_HOURS(n) (n / 3600000)

#ifndef EVERY_N_MILLIS
// EVERY_N_MILLIS(1000)
// {
// do something every 1000 miliseconds
// }
#define EVERY_N_MILLIS(n) I_EVERY_N_MILLIS(CONCAT(_timer_, __COUNTER__), n)
#define I_EVERY_N_MILLIS(name, n)             \
    static SimpleTimer name = SimpleTimer(n); \
    if (name.ready())

// EVERY_N_MICROS(1000)
// {
// do something every 1000 microseconds
// }
#define EVERY_N_MICROS(n) I_EVERY_N_MICROS(CONCAT(_timer_, __COUNTER__), n)
#define I_EVERY_N_MICROS(name, n)             \
    static TimerMicros name = TimerMicros(n); \
    if (name.ready())

// Join two symbols together
#define CONCAT(x, y) I_CONCAT(x, y)
#define I_CONCAT(x, y) x##y
#endif