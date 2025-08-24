#pragma once

#include <chrono>

#define VELOCITY_MEASURE_PIN_1 16
#define VELOCITY_MEASURE_PIN_2 4

const double WHEEL_CIRCUMFERENCE = 1.4444;

class HallSensor {
public:
    HallSensor();
    void begin();
    void loop();

    float current_velocity;


private:

    static void addRotationSensor();
    void calculateVelocity();

static volatile long rotation_sensor1;
static volatile long rotation_sensor2;

static std::chrono::steady_clock::time_point interrupt_time;

const std::chrono::milliseconds measure_time_ms{1500};



};