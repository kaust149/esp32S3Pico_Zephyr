/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "keyboard.h"

#include <lvgl.h>
#include "terminal.h"

#include <lvgl_input_device.h>
#define SLEEP_TIME_MS   3//100

static uint32_t count;

static void lv_btn_click_callback(lv_event_t *e)
{
	ARG_UNUSED(e);

	count = 0;
}


int main(void)
{

	char count_str[11] = {0};
	const struct device *display_dev;
	lv_obj_t *hello_world_label;
	lv_obj_t *count_label;
	int ret;

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		printf("Device not ready, aborting test");
		return 0;
	}

	char keyVal = 0;
	printf("Hello World! %s\n", CONFIG_BOARD_TARGET);

	display_blanking_off(display_dev);
    printf("Display ready %d %d\n",lv_disp_get_hor_res(NULL),lv_disp_get_ver_res(NULL));
	
    // static uint16_t buf[320 * 20]; // 20 lines

    // for (int i = 0; i < ARRAY_SIZE(buf); i++) {
    //     buf[i] = 0x001F; // RED (RGB565)
    // }

    // struct display_buffer_descriptor desc = {
    //     .width = 320,
    //     .height = 20,
    //     .pitch = 320,
    //     .buf_size = sizeof(buf),
    // };

    // display_write(display_dev, 0, 0, &desc, buf);
	// lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x123456), 0);

	// while(1);

	// const struct device *display = DEVICE_DT_GET(DT_NODELABEL(ili9341));

    // if (!device_is_ready(display)) 
	// {
	// 	printf("Display Device is Not ready\n");
    //     return;
    // }
	// else
	// {
	// 	printf("Display Device is ready for use\n");
	// }
    // display_blanking_off(display);
	// lv_init();

    lv_obj_t *scr = lv_scr_act();
    terminal_init(scr);
	terminal_init(scr);
	keyboard_init();
	sb_write_lcd_backlight(250);
	// if (IS_ENABLED(CONFIG_LV_Z_POINTER_INPUT)) {
	// 	lv_obj_t *hello_world_button;

	// 	hello_world_button = lv_button_create(lv_screen_active());
	// 	lv_obj_align(hello_world_button, LV_ALIGN_CENTER, 0, -15);
	// 	lv_obj_add_event_cb(hello_world_button, lv_btn_click_callback, LV_EVENT_CLICKED,
	// 			    NULL);
	// 	hello_world_label = lv_label_create(hello_world_button);
	// } else {
	// 	hello_world_label = lv_label_create(lv_screen_active());
	// }

	// lv_label_set_text(hello_world_label, "Hello world!");
	// lv_obj_align(hello_world_label, LV_ALIGN_CENTER, 0, 0);

	// count_label = lv_label_create(lv_screen_active());
	// lv_obj_align(count_label, LV_ALIGN_BOTTOM_MID, 0, 0);

	// lv_timer_handler();
	// ret = display_blanking_off(display_dev);
	// if (ret < 0 && ret != -ENOSYS) {
	// 	printf("Failed to turn blanking off (error %d)", ret);
	// 	return 0;
	// }

	while (1) 
	{
		if ((count % 100) == 0U) {
			sprintf(count_str, "%d", count/100U);
			printf("%s\n",count_str);
		}

		lv_timer_handler();
		++count;
// /***********This Section Is working**********
		if (sb_available())
		{
    		keyboard_poll();
		}
		if(keyboard_key_available())
		{
			keyVal = keyboard_get_key();
			terminal_handle_key(keyVal);
			printf("%c",keyVal);
		}
// **********This Section Is working***********/

		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}
