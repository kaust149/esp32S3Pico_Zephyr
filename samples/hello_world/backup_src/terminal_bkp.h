#pragma once
#include <lvgl.h>

void terminal_init(lv_obj_t *parent);
void terminal_putchar(char c);
void terminal_print(const char *str);
void terminal_prompt(void);
void terminal_clear(void);
void terminal_handle_key(uint32_t key);

