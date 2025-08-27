#include "hall_sensor.hpp"
#include "gpiod_platform.hpp"
#include <thread>
#include <chrono>
#include <atomic>
#include <iostream>
HallSensor hall_sesnor_instance;

volatile long HallSensor::rotation_sensor1 = 0;
volatile long HallSensor::rotation_sensor2 = 0;

std::chrono::steady_clock::time_point HallSensor::interrupt_time;

HallSensor::HallSensor() : current_velocity(0.0f) {
    last_velocity_measure = std::chrono::steady_clock::now();
}

void HallSensor::begin(){
    last_velocity_measure = std::chrono::steady_clock::now();

    gpiod_chip* chip = gpiod_chip_open_by_name("gpiochip0");
    if (!chip) {
        std::cerr << "Failed to open GPIO chip";
    };

    gpiod_line* line = gpiod_chip_get_line(chip, VELOCITY_MEASURE_PIN_2);
if (!line || gpiod_line_request_rising_edge_events(line, "hall_sensor") < 0) {
        std::cerr << "Failed to request rising edge events for GPIO line\n";
        return;
    }

    std::thread([line]() {
        struct gpiod_line_event event;
        while (true) {
            int ret = gpiod_line_event_wait(line, nullptr);
            if (ret > 0 && gpiod_line_event_read(line, &event) == 0) {
                HallSensor::addRotationSensor();
            }
        }
    }).detach();
}

void HallSensor::addRotationSensor(){
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - interrupt_time).count();
    if (duration > 30) { // Debounce time of 30 ms
        rotation_sensor2++;
        interrupt_time = now;
    }
}

void HallSensor::calculateVelocity(){
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - last_velocity_measure);

    if (elapsed_ms >= measure_time_ms) {
        long rotations = rotation_sensor2;
        rotation_sensor2 = 0;

        float distance = rotation_sensor2 * WHEEL_CIRCUMFERENCE;
        float local_time = elapsed_ms.count() / 1000.0f; 
        float velocity = distance / local_time; 
       
        current_velocity = velocity * 3.6f;
        rotation_sensor2 = 0;
        last_velocity_measure = currentTime;
    }
}
void HallSensor::loop(){
    calculateVelocity();
}