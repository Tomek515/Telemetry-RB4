#include "hall_sensor.hpp"
#include <iostream>

HallSensor hall_sensor_instance;

static inline long long now_ms_steady() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

HallSensor::HallSensor() : current_velocity(0.0f) {
    last_velocity_measure = std::chrono::steady_clock::now();
}

HallSensor::~HallSensor() {
    stop();
}

void HallSensor::begin() {
    if (running.load()) return;

    last_velocity_measure = std::chrono::steady_clock::now();
    rotations.store(0, std::memory_order_relaxed);
    last_interrupt_ms.store(0, std::memory_order_relaxed);

    chip = gpiod_chip_open_by_name("gpiochip0");
    if (!chip) {
        std::cerr << "[HallSensor] Nie udało się otworzyć gpiochip0\n";
        return;
    }

    line = gpiod_chip_get_line(chip, VELOCITY_MEASURE_PIN_2);
    if (!line) {
        std::cerr << "[HallSensor] Nie udało się pobrać linii GPIO " << VELOCITY_MEASURE_PIN_2 << "\n";
        gpiod_chip_close(chip); chip = nullptr;
        return;
    }

    if (gpiod_line_request_rising_edge_events(line, "hall_sensor") < 0) {
        std::cerr << "[HallSensor] Nie udało się zarejestrować zdarzeń zbocza narastającego\n";
        gpiod_line_release(line); line = nullptr;
        gpiod_chip_close(chip); chip = nullptr;
        return;
    }

    running.store(true, std::memory_order_release);

    event_thread = std::thread([this]() {
        while (running.load(std::memory_order_acquire)) {
            const timespec ts { .tv_sec = 0, .tv_nsec = 500'000'000 }; 
            int ret = gpiod_line_event_wait(line, &ts);

            if (!running.load(std::memory_order_acquire)) break;

            if (ret < 0) {
                std::cerr << "[HallSensor] Błąd event_wait\n";
                continue;
            }
            if (ret == 0) {
            
                continue;
            }

            gpiod_line_event ev{};
            if (gpiod_line_event_read(line, &ev) == 0) {
                if (ev.event_type == GPIOD_LINE_EVENT_RISING_EDGE) {
                    onEdgeEvent();
                }
            }
        }
    });
}

void HallSensor::stop() {
    if (!running.load(std::memory_order_acquire)) return;

    running.store(false, std::memory_order_release);

    if (event_thread.joinable()) {
        event_thread.join();
    }

    if (line) {
        gpiod_line_release(line);
        line = nullptr;
    }
    if (chip) {
        gpiod_chip_close(chip);
        chip = nullptr;
    }
}

void HallSensor::onEdgeEvent() {
    const long long t_now = now_ms_steady();
    const long long t_prev = last_interrupt_ms.load(std::memory_order_relaxed);

    if (t_now - t_prev > debounce_ms) {
        rotations.fetch_add(1, std::memory_order_relaxed);
        last_interrupt_ms.store(t_now, std::memory_order_relaxed);
    }
}

void HallSensor::calculateVelocity() {
    using namespace std::chrono;

    const auto currentTime = steady_clock::now();
    const auto elapsed_ms = duration_cast<milliseconds>(currentTime - last_velocity_measure);

    if (elapsed_ms >= measure_time_ms) {
        const long pulses = rotations.exchange(0, std::memory_order_acq_rel);

        const float rotations_f = static_cast<float>(pulses);

        const float distance_m = rotations_f * static_cast<float>(WHEEL_CIRCUMFERENCE);
        const float dt_s = elapsed_ms.count() / 1000.0f;

        const float velocity_ms = (dt_s > 0.0f) ? (distance_m / dt_s) : 0.0f;

        current_velocity = velocity_ms * 3.6f;

        last_velocity_measure = currentTime;
    }
}

void HallSensor::loop() {
    calculateVelocity();
}
