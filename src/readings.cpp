#include "readings.hpp"
#include "hall_sensor.hpp"
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <gpiod.h>
#include <string>
#include <sstream>
#include <iomanip>

#define SPI_DEVICE "/dev/spidev0.0"
#define CS_GPIO_CHIP "/dev/gpiochip0"
#define CS_PIN_1 5
#define CS_PIN_1 6
#define CS_PIN_1 22
#define CS_PIN_1 7
#define CS_PIN_1 12

const int cs_pins[5] = {CS_PIN_1, CS_PIN_2, CS_PIN_3, CS_PIN_4, CS_PIN_5};
const int num_devices = sizeof(cs_pins) / sizeof(cs_pins[0]);

const float vRef = 5.0;
const float constVoltageScalling = 0.96;
const float scaleVoltage = (10+1.2)/1.2;

const float zeroCurrentVoltage = 2.551;
const float sensitivityACS = 0.040;
const float constCurrentScalling = 1.06;

int spid_fd = -1;
gpiod_line_bulk cs_lines;
gpiod_chip* chip = nullptr;

void readings_init() {
    memset(&cs_lines, 0, sizeof(cs_lines));

    spi_fd = open(SPI_DEVICE, O_RDWR);

    if (spi_fd < 0) {
        std::cerr << "Failed to open SPI device" << std::endl;
        return;
    }

    uint8_t mode = SPI_MODE_0 | SPI_NO_CS;
    uint32_t speed = 1000000;
    
    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    chip = gpiod_chip_open(CS_GPIO_CHIP);
    if (!chip) {
        std::cerr << "Failed to open GPIO chip" << std::endl;
        
        return;
    }

     for(int pin: cs_pins){
        gpiod_line* line = gpiod_chip_get_line(chip, pin);
        if(pin != 7 && (line == nullptr || gpiod_line_request_output(line, "spi-cs", 1) < 0)){
            std::cerr << "Failed to request GPIO line for CS pin " << pin << "\n";
            gpiod_chip_close(chip);
            return;
        }
        gpiod_line_bulk_add(&cs_lines, line);
    }
}

void selectCS(int index){
    gpiod_line_set_value(cs_lines.lines[index], 0);
}
void deselectCS(int index){
    gpiod_line_set_value(cs_lines.lines[index], 1);
}

float adcToVoltage(uint16_t adcValue){
    return ((float)adcValue * vRef / 4095.0) * constVoltageScalling * scaleVoltage;
}

float adcToCurrent(uint16_t adcValue){
    return (((float)adcValue * vRef / 4095.0) - zeroCurrentVoltage) / sensitivityACS * constCurrentScalling;
}
