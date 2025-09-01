#include "readings.hpp"
#include "sd_card_save.hpp"
#include "hall_sensor.hpp"
#include "mqtt_communication.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>

static unsigned long millis() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return static_cast<unsigned long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count()
    );
}

static std::atomic<bool> keep_running{true};

static void handle_sigint(int) {
    keep_running.store(false);
}

int main() {
    std::signal(SIGINT, handle_sigint);
    std::signal(SIGTERM, handle_sigint);

    hall_sensor_instance.begin();
    readings_init();
    sd_card_save_init();
    server_communication_instance.begin(); 

    const unsigned long measurementInterval = 10;    
    const unsigned long flushInterval       = 1000; 

    unsigned long lastMeasurementTime = millis();
    unsigned long lastFlushTime       = lastMeasurementTime;

    while (keep_running.load()) {
        unsigned long now = millis();

        if (now - lastMeasurementTime >= measurementInterval) {
            lastMeasurementTime = now;

            Readings reading = exec_reading();               
            std::cout << readingsToString(reading) << "\n";  
            sd_card_save_append(reading);                   

            if (now - lastFlushTime >= flushInterval) {
                lastFlushTime = now;
                server_communication_instance.publishMqttMessage(readingsToString(reading));
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

   
    sd_card_save_close();         
    hall_sensor_instance.stop();

   
    return 0;
}
