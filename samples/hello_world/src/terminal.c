#include "terminal.h"
#include <string.h>
#include <lvgl.h>
#include "ui_shell.h"

#include <zephyr/shell/lvgl_shell.h>

// lv_obj_t *ta;
static char input_buf[128];
static uint8_t input_len;

#define PROMPT "> "

#define TERM_COLS 40
#define TERM_ROWS 20
#define CHAR_W    8
#define CHAR_H    16
#define SCREEN_W    (TERM_COLS * CHAR_W)
#define SCREEN_H    (TERM_ROWS * CHAR_H)

static char term_buf[TERM_ROWS][TERM_COLS];
static uint8_t cur_x;
static uint8_t cur_y;

static lv_obj_t *rows[TERM_ROWS];

static lv_obj_t *canvas;
static lv_color_t *cbuf;

static lv_style_t text_style;

/* Shell instance */

// /* Output hook */
// static int shell_lvgl_write(const struct shell *sh,
//                             const char *data, size_t len)
// {
//     ARG_UNUSED(sh);

//     for (size_t i = 0; i < len; i++) {
//         terminal_putchar(data[i]);
//     }
//     return len;
// }

// /* Backend API */
// static const struct shell_transport_api shell_lvgl_transport = {
//     .write = shell_lvgl_write,
// };

// /* Dummy transport context */
// static struct shell_transport shell_lvgl_ctx = {
//     .api = &shell_lvgl_transport,
// };

// /* Shell instance */
// SHELL_DEFINE(lvgl_shell,
//              "lvgl",
//              shell_lvgl_write,
//              shell_lvgl_write,
//              &shell_lvgl_ctx,
//              CONFIG_SHELL_STACK_SIZE);

//  /* Dummy read function (we call it when a key is pressed) */
// static void lvgl_shell_feed_char(uint8_t c)
// {
//     struct shell *sh = &lvgl_shell; // the shell defined by SHELL_DEFINE
//     // sh->ctx->api->input(sh->ctx, &c, 1); // feed one char
//     // shell_input(sh, &c, 1);
// }
/* Scroll terminal up by one line */
static void term_scroll(void)
{
    memmove(term_buf[0],
            term_buf[1],
            (TERM_ROWS - 1) * TERM_COLS);

    memset(term_buf[TERM_ROWS - 1], ' ', TERM_COLS);
    cur_y = TERM_ROWS - 1;
}

/* Put one character */
void terminal_putc(char c)
{
    if (c == '\r') {
        cur_x = 0;
        cur_y++;
        if (cur_y >= TERM_ROWS) {
            term_scroll();
        }
        return;
    }

    if (c == '\n') {
        cur_x = 0;
        cur_y++;
        if (cur_y >= TERM_ROWS) {
            term_scroll();
        }
        return;
    }

    if (c == '\b' || c == 0x7F) {
        if (cur_x > 0) {
            cur_x--;
            term_buf[cur_y][cur_x] = ' ';
        }
        return;
    }

    term_buf[cur_y][cur_x] = c;
    cur_x++;

    if (cur_x >= TERM_COLS) {
        cur_x = 0;
        cur_y++;
        if (cur_y >= TERM_ROWS) {
            term_scroll();
        }
    }
}

/* Write string */
void terminal_puts(const char *s)
{
    while (*s) {
        terminal_putc(*s++);
    }
}

/* Render terminal grid */
// void terminal_render(void)
// {
//     lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);

//     for (int y = 0; y < TERM_ROWS; y++) {
//         lv_canvas_draw_text(
//             canvas,
//             0,
//             y * CHAR_H,
//             TERM_COLS * CHAR_W,
//             &text_style,
//             term_buf[y]);
//     }
// }
void terminal_render(void)
{
    for (int y = 0; y < TERM_ROWS; y++) {
        lv_label_set_text_fmt(
            rows[y], "%.*s", TERM_COLS, term_buf[y]);
    }
}
/* Init terminal */
// void terminal_init(void)
// {
//     memset(term_buf, ' ', sizeof(term_buf));
//     cur_x = 0;
//     cur_y = 0;

//     lv_display_set_resolution(lv_display_get_default(), 320, 320);
//     lv_style_init(&text_style);
//     lv_style_set_text_font(&text_style, &lv_font_unscii_16);
//     lv_style_set_text_color(&text_style, lv_color_white());
//     lv_style_set_bg_opa(&text_style, LV_OPA_TRANSP);

//     /* Allocate canvas buffer */
//     // cbuf = lv_mem_alloc(
//     //     LV_CANVAS_BUF_SIZE_TRUE_COLOR(SCREEN_W, SCREEN_H));

//     static lv_color_t cbuf[SCREEN_W * SCREEN_H];

//     canvas = lv_canvas_create(lv_scr_act());
//     lv_canvas_set_buffer(
//         canvas,
//         cbuf,
//         SCREEN_W,
//         SCREEN_H,
//         LV_COLOR_FORMAT_RGB565);

//     lv_obj_center(canvas);

//     terminal_puts("PicoCalc Terminal 40x20\n");
//     terminal_puts("-----------------------\n");
//     terminal_puts("> ");

//     terminal_render();
// }

// void terminal_init(void)
// {
//     memset(term_buf, ' ', sizeof(term_buf));
//     cur_x = 0;
//     cur_y = 0;

//     lv_display_set_resolution(lv_display_get_default(), 320, 320);

//     lv_obj_t *scr = lv_scr_act();
//     lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

//     for (int y = 0; y < TERM_ROWS; y++) {
//         rows[y] = lv_label_create(scr);
//         lv_label_set_long_mode(rows[y], LV_LABEL_LONG_CLIP);
//         lv_obj_set_width(rows[y], 320);
//         lv_obj_set_pos(rows[y], 0, y * CHAR_H);

//         lv_obj_set_style_text_font(
//             rows[y], &lv_font_unscii_16, 0);
//         lv_obj_set_style_text_color(
//             rows[y], lv_color_white(), 0);

//         lv_label_set_text_fmt(rows[y], "%.*s",
//                               TERM_COLS, term_buf[y]);
//     }

//     terminal_puts("PicoCalc Terminal 40x20\n");
//     terminal_puts("-----------------------\n");
//     terminal_puts("> ");
//     terminal_render();
// }

void on_key(char ascii)
{
    terminal_putc(ascii);
    terminal_render();
}

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

    lv_obj_set_style_text_font(ta, &lv_font_unscii_8, 0);//lv_font_montserrat_14 lv_font_unscii_16 lv_font_unscii_8

    terminal_prompt();


    // shell_start(&lvgl_shell);
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
        shell_feed_test(input_buf);
        /* ---- Command handling ---- */
        if (!strcmp(input_buf, "clear")) {
            terminal_clear();
            return;
        }

        if (!strcmp(input_buf, "calc")) {
            terminal_print("Calculator mode coming soon\n");
        } else {
            // terminal_print("Unknown command\n");
        }

        terminal_prompt();
        return;
    }

    if (key == 0x08) { /* Backspace */
        if (input_len > 0) {
            input_len--;
            lv_textarea_delete_char(ta);
        }
        return;
    }

    if (input_len < sizeof(input_buf) - 1) {
        input_buf[input_len++] = key;
        terminal_putchar(key);
    }
}

// void terminal_handle_key(uint32_t key)
// {
//     if (key == '\r' || key == '\n') {
//         lvgl_shell_feed_char('\n');
//         return;
//     }

//     if (key == 0x08 || key == 0x7F) {
//         lvgl_shell_feed_char('\b');
//         return;
//     }

//     char c = (char)key;
//     lvgl_shell_feed_char(c);
// }
