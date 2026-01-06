#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct stdio_driver stdio_driver_t;

struct stdio_driver {
    void (*out_chars)(const char *buf, int len);
    void (*out_flush)(void);
    int (*in_chars)(char *buf, int len);
    void (*set_chars_available_callback)(void (*fn)(void*), void *param);
    stdio_driver_t *next;
#if PICO_STDIO_ENABLE_CRLF_SUPPORT
    bool last_ended_with_cr;
    bool crlf_enabled;
#endif
};

typedef struct repeating_timer repeating_timer_t;

typedef bool (*repeating_timer_callback_t)(repeating_timer_t *rt);

struct repeating_timer {
    int64_t delay_us;
    void *pool;
    char alarm_id;
    repeating_timer_callback_t callback;
    void *user_data;
};
typedef void (*led_callback_t)(uint8_t);

extern stdio_driver_t picocalc_stdio_driver;

// Function prototypes
void picocalc_chars_available_notify(void);
void picocalc_init(void);
