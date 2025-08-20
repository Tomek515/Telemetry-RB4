#include "hall_sensor.hpp"
#include "gpiod_platform.hpp"
#include <thread>
#include <chrono>
#include <atomic>

HallSensor hall_sesnor_instance;

volatile long HallSensor::rotation_sensor1 = 0;
volatile long HallSensor::rotation_sensor2 = 0;

std::chrono::steady_clock::time_point HallSensor::interrupt_time;

HallSensor::HallSensor() : current_velocity(0.0f) {
    last_velocity_measure = std::chrono::steady_clock::now();
}
