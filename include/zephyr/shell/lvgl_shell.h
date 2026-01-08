#ifndef LVGL_SHELL_H__
#define LVGL_SHELL_H__

#include <zephyr/shell/shell.h>
#include <lvgl.h>

#pragma once

int lvgl_shell_write(const char *fmt, va_list args, char color);



void terminal_init(lv_obj_t *parent);
void terminal_putchar(char c);
void terminal_print(const char *str);
void terminal_prompt(void);
void terminal_clear(void);
void terminal_handle_key(uint32_t key);

// void terminal_init(void);
void terminal_putc(char c);
void terminal_puts(const char *s);
void terminal_render(void);
void on_key(char ascii);

extern lv_obj_t *ta;






#endif
