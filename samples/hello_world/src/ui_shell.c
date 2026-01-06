#include <zephyr/shell/shell.h>
#include "ui_shell.h"
#include "terminal.h"
// struct shell shell_ui;
#define SHELL_DEFAULT_BACKEND_CONFIG_FLAGS                              \
{                                                                       \
        .insert_mode    = 0,                                            \
        .echo           = 1,                                            \
        .obscure        = IS_ENABLED(CONFIG_SHELL_START_OBSCURED),      \
        .mode_delete    = 1,                                            \
        .use_colors     = 1,                                            \
        .use_vt100      = 1,                                            \
};

struct shell_backend_config_flags defaulBackendFlag = SHELL_DEFAULT_BACKEND_CONFIG_FLAGS;
static struct shell_transport lvgl_transport;
static struct shell *lvgl_shell;

static char line_buf[128];
static int line_len;
struct shell *ui_shell;
static void shell_print_cb(const struct shell *shell,
                           const char *fmt, va_list args)
{
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, args);
    printf("callbacked");
    terminal_print(buf);
}

/* Transport API */
static const struct shell_transport_api shell_ui_transport_api = {
    .write = shell_print_cb,
};

/* Transport interface */
static struct shell_transport shell_ui_transport = {
    .api = &shell_ui_transport_api,
};

/*
 * CORRECT for your Zephyr:
 *  - 6 arguments
 *  - no stack size
 *  - no shell_init()
 */
// SHELL_DEFINE(shell_ui,
//              "kabootar> ",
//              &shell_ui_transport,
//              128,          /* log queue size */
//              0,          /* log timeout */
//              SHELL_FLAG_CRLF_DEFAULT); //SHELL_FLAG_DEFAULT

void shell_feed_char(char c)
{
    if (c == '\r' || c == '\n') {
        line_buf[line_len] = 0;
        printf("1 %c\n",c);
        shell_execute_cmd(shell_backend_uart_get_ptr(), line_buf);//shell_backend_uart_get_ptr
        // shell_execute_cmd(lvgl_shell, line_buf);
        line_len = 0;
    } else if (c == '\b') {
        printf("2 %c\n",c);
        if (line_len > 0) line_len--;
    } else {
        if (line_len < sizeof(line_buf) - 1) {
            printf("3 %c\n",c);
            line_buf[line_len++] = c;
        }
    }
}

void shell_feed_test(char *c)
{
    shell_execute_cmd(shell_backend_uart_get_ptr(), c);//shell_backend_uart_get_ptr
    // shell_execute_cmd(lvgl_shell, c);
    // terminal_print(ui_shell->fprintf_ctx->buffer);
    // terminal_print("test here");
    // printf("%d\n",ui_shell->fprintf_ctx->buffer_size);
}

void initShell(void)
{
    // shell_init(shell_backend_uart_get_ptr(), &shell_ui_transport,defaulBackendFlag,false,0);
    // int shell_init(*sh, *transport_config, cfg_flags, log_backend,  init_log_level);
}

int startShell(void)
{
//    ui_shell = shell_backend_uart_get_ptr();
//    ui_shell->iface->api = &shell_ui_transport_api;
//    termBuff1 = ui_shell->fprintf_ctx->buffer;
   return shell_start(shell_backend_uart_get_ptr());//shell_backend_uart_get_ptr
//    return shell_start(lvgl_shell);
}

void checkShellReady(void)
{
    int shellReadyCntr = 0;
    while (!shell_ready(shell_backend_uart_get_ptr())) { //shell_backend_uart_get_ptr
    // while (!shell_ready(lvgl_shell)) {

    printf("Shell Ready cntr %d\r\n",shellReadyCntr++);
    if(shellReadyCntr > 10)
    {
        // break;
    }
    k_sleep(K_MSEC(100));
}
printf("Shell Ready\r\n");
}

// static int lvgl_shell_write(const struct shell *sh,
//                             const char *data, size_t len, size_t *count)
// {
//     static char buf[256];
//     size_t actLen;
//     actLen = strlen(data);
//     size_t l = MIN(actLen, sizeof(buf) - 1);
//     memcpy(buf, data, l);
//     buf[l] = 0;
//     printf("lvgl_shell_write %s\r\n",buf);
//     lv_textarea_add_text(ta, buf);
//     lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);

//     return l;
// }

// static const struct shell_transport_api lvgl_transport_api = {
//     .write = lvgl_shell_write,
// };

// /* Transport interface */
// static struct shell_transport lvgl_transport = {
//     .api = &lvgl_transport_api,
// };

// void shell_lvgl_init(void)
// {
//     // shell_transport_init(&lvgl_transport, &lvgl_transport_api, NULL);
//     char retval = 0,err = 0;
//     // srtuct shell *sh;
//     retval = shell_init(shell_backend_uart_get_ptr(), //lvgl_shell
//                &lvgl_transport,
//                defaulBackendFlag,
//                false,
//                0);

//     printf("Return for shell_init %d\r\n",retval);
//     lvgl_shell = shell_backend_uart_get_ptr();
//     err = lvgl_shell->iface->api->enable(lvgl_shell->iface, true);
// 	if (err != 0) {
// 		return;
// 	}
// }