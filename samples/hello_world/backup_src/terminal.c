// #include "terminal.h"
// #include <string.h>
// #include <lvgl.h>
// #include "ui_shell.h"

// #include <zephyr/shell/lvgl_shell.h>

// // lv_obj_t *ta;
// static char input_buf[128];
// static uint8_t input_len;

// #define PROMPT "> "

// #define TERM_COLS 40
// #define TERM_ROWS 20
// #define CHAR_W    8
// #define CHAR_H    16
// #define SCREEN_W    (TERM_COLS * CHAR_W)
// #define SCREEN_H    (TERM_ROWS * CHAR_H)

// static char term_buf[TERM_ROWS][TERM_COLS];
// static uint8_t cur_x; 
// static uint8_t cur_y;

// static lv_obj_t *rows[TERM_ROWS];

// static lv_obj_t *canvas;
// static lv_color_t *cbuf;

// static lv_style_t text_style;

// /* Scroll terminal up by one line */
// static void term_scroll(void)
// {
//     memmove(term_buf[0],
//             term_buf[1],
//             (TERM_ROWS - 1) * TERM_COLS);

//     memset(term_buf[TERM_ROWS - 1], ' ', TERM_COLS);
//     cur_y = TERM_ROWS - 1;
// }

// /* Put one character */
// void terminal_putc(char c)
// {
//     if (c == '\r') {
//         cur_x = 0;
//         cur_y++;
//         if (cur_y >= TERM_ROWS) {
//             term_scroll();
//         }
//         return;
//     }

//     if (c == '\n') {
//         cur_x = 0;
//         cur_y++;
//         if (cur_y >= TERM_ROWS) {
//             term_scroll();
//         }
//         return;
//     }

//     if (c == '\b' || c == 0x7F) {
//         if (cur_x > 0) {
//             cur_x--;
//             term_buf[cur_y][cur_x] = ' ';
//         }
//         return;
//     }

//     term_buf[cur_y][cur_x] = c;
//     cur_x++;

//     if (cur_x >= TERM_COLS) {
//         cur_x = 0;
//         cur_y++;
//         if (cur_y >= TERM_ROWS) {
//             term_scroll();
//         }
//     }
// }

// /* Write string */
// void terminal_puts(const char *s)
// {
//     while (*s) {
//         terminal_putc(*s++);
//     }
// }

// void terminal_render(void)
// {
//     for (int y = 0; y < TERM_ROWS; y++) {
//         lv_label_set_text_fmt(
//             rows[y], "%.*s", TERM_COLS, term_buf[y]);
//     }
// }

// void on_key(char ascii)
// {
//     terminal_putc(ascii);
//     terminal_render();
// }

// void terminal_init(lv_obj_t *parent)
// {
//     lv_display_set_resolution(lv_display_get_default(), 320, 320);
//     ta = lv_textarea_create(parent);
//     lv_obj_set_size(ta,320,lv_disp_get_ver_res(NULL));
//     lv_textarea_set_text(ta, "");
//     lv_textarea_set_cursor_click_pos(ta, false);
//     lv_textarea_set_max_length(ta, 4096);
//     lv_textarea_set_one_line(ta, false);
//     lv_textarea_set_password_mode(ta, false);

//     lv_obj_set_style_text_font(ta, &lv_font_unscii_8, 0);//lv_font_montserrat_14 lv_font_unscii_16 lv_font_unscii_8

//     terminal_prompt();


//     // shell_start(&lvgl_shell);
// }


// void terminal_prompt(void)
// {
//     terminal_print(PROMPT);
//     input_len = 0;
// }

// void terminal_clear(void)
// {
//     lv_textarea_set_text(ta, "");
//     terminal_prompt();
// }

// void terminal_putchar(char c)
// {
//     char s[2] = {c, 0};
//     lv_textarea_add_text(ta, s);
//     lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
// }

// void terminal_print(const char *str)
// {
//     lv_textarea_add_text(ta, str);
//     lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
// }
   
// void terminal_handle_key(uint32_t key)
// {
//     if (key == '\n' || key == '\r') {
//         terminal_putchar('\n');

//         input_buf[input_len] = 0;
//         shell_feed_test(input_buf);
//         /* ---- Command handling ---- */
//         if (!strcmp(input_buf, "clear")) {
//             terminal_clear();
//             return;
//         }

//         if (!strcmp(input_buf, "calc")) {
//             terminal_print("Calculator mode coming soon\n");
//         } else {
//             // terminal_print("Unknown command\n");
//         }

//         terminal_prompt();
//         return;
//     }

//     if (key == 0x08) { /* Backspace */
//         if (input_len > 0) {
//             input_len--;
//             lv_textarea_delete_char(ta);
//         }
//         return;
//     }

//     if (input_len < sizeof(input_buf) - 1) {
//         input_buf[input_len++] = key;
//         terminal_putchar(key);
//     }
// }