#include <lvgl.h>

lv_obj_t *ta;

int lvgl_shell_write(const char *fmt, va_list args, char color)
{
    static char buf[2560];
    vsnprintf(buf, sizeof(buf), fmt, args);
    lv_textarea_add_text(ta, buf);
    lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);

    return strlen(buf);
}
