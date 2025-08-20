#pragma once

#if defined(__linux__)
#include <gpiod.h>
#else 

struct gpiod_chip {};
struct gpiod_line {};
struct gpiod_line_request_config {
    const char *consumer;
    int request_type;
    int flags;
};
  #ifndef GPIOD_LINE_REQUEST_DIRECTION_OUTPUT
  #define GPIOD_LINE_REQUEST_DIRECTION_OUTPUT 1
  #endif

  inline gpiod_chip* gpiod_chip_open(const char*) { return nullptr; }
  inline void        gpiod_chip_close(gpiod_chip*) {}
  inline gpiod_line* gpiod_chip_get_line(gpiod_chip*, int) { return nullptr; }
  inline int         gpiod_line_request(gpiod_line*, const gpiod_line_request_config*, int){ return 0; }
  inline void        gpiod_line_release(gpiod_line*) {}
  inline int         gpiod_line_set_value(gpiod_line*, int){ return 0; }
#endif