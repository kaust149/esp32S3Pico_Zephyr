#include "terminal.h"
#include <string.h>

static lv_obj_t *ta;
static char input_buf[128];
static uint8_t input_len;

#define PROMPT "> "

void terminal_init(lv_obj_t *parent)
{
    lv_display_set_resolution(lv_display_get_default(), 320, 320);
    ta = lv_textarea_create(parent);
    lv_obj_set_size(ta,320,lv_disp_get_ver_res(NULL));
    lv_textarea_set_text(ta, "");
    lv_textarea_set_cursor_click_pos(ta, false);
    lv_textarea_set_max_length(ta, 4096);
    lv_textarea_set_one_line(ta, false);
    lv_textarea_set_password_mode(ta, false);

    lv_obj_set_style_text_font(
        ta, &lv_font_montserrat_14, 0);

    terminal_prompt();
}

void terminal_prompt(void)
{
    terminal_print(PROMPT);
    input_len = 0;
}

void terminal_clear(void)
{
    lv_textarea_set_text(ta, "");
    terminal_prompt();
}

void terminal_putchar(char c)
{
    char s[2] = {c, 0};
    lv_textarea_add_text(ta, s);
    lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
}

void terminal_print(const char *str)
{
    lv_textarea_add_text(ta, str);
    lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
}

void terminal_handle_key(uint32_t key)
{
    if (key == '\n' || key == '\r') {
        terminal_putchar('\n');

        input_buf[input_len] = 0;

        /* ---- Command handling ---- */
        if (!strcmp(input_buf, "clear")) {
            terminal_clear();
            return;
        }

        if (!strcmp(input_buf, "calc")) {
            terminal_print("Calculator mode coming soon\n");
        } else {
            terminal_print("Unknown command\n");
        }

        terminal_prompt();
        return;
    }

    if (key == 0x08) { /* Backspace */
        if (input_len > 0) {
            input_len--;
            // lv_textarea_del_char(ta);
        }
        return;
    }

    if (input_len < sizeof(input_buf) - 1) {
        input_buf[input_len++] = key;
        terminal_putchar(key);
    }
}
