#pragma once

#include <chrono>
#include <thread>
#include <atomic>


#include "gpiod_platform.hpp"

#define VELOCITY_MEASURE_PIN_1 16
#define VELOCITY_MEASURE_PIN_2 4

// Obwód koła w metrach
constexpr double WHEEL_CIRCUMFERENCE = 1.4444;

class HallSensor {
public:
    HallSensor();
    ~HallSensor();

    void begin();

    void stop();

    void loop();

    float current_velocity;

    HallSensor(const HallSensor&) = delete;
    HallSensor& operator=(const HallSensor&) = delete;

private:
    void onEdgeEvent();

    void calculateVelocity();

    gpiod_chip* chip = nullptr;
    gpiod_line* line = nullptr;

    std::thread event_thread;
    std::atomic<bool> running{false};

    std::atomic<long> rotations{0};                 
    std::atomic<long long> last_interrupt_ms{0};   
    std::chrono::steady_clock::time_point last_velocity_measure;
    const std::chrono::milliseconds measure_time_ms{1500};
    const int debounce_ms = 30; 
};

extern HallSensor hall_sensor_instance;
