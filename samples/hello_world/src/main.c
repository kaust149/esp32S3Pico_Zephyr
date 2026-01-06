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
#include <zephyr/version.h>
#include "keyboard.h"

#include <lvgl.h>
#include "terminal.h"

#include <lvgl_input_device.h>

#include <zephyr/shell/shell.h>

#include <zephyr/drivers/uart.h>
#include "ui_shell.h"

#define CONFIG_SHELL_GETOPT

#define SLEEP_TIME_MS   10//100

static uint32_t count;

// static const struct device *uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

static void lv_btn_click_callback(lv_event_t *e)
{
	ARG_UNUSED(e);

	count = 0;
}

static int cmd_version(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(sh, "Zephyr version %s", KERNEL_VERSION_STRING);

	return NULL;
}

static int cmd_demo_ping(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(sh, "pong");

	return 0;
}

static int cmd_demo_board(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(sh, CONFIG_BOARD);

	return 0;
}

#if defined CONFIG_SHELL_GETOPT
/* Thread save usage */
static int cmd_demo_getopt_ts(const struct shell *sh, size_t argc,
			      char **argv)
{
	struct sys_getopt_state *state;
	char *cvalue = NULL;
	int aflag = 0;
	int bflag = 0;
	int c;

	while ((c = sys_getopt(argc, argv, "abhc:")) != -1) {
		state = sys_getopt_state_get();
		switch (c) {
		case 'a':
			aflag = 1;
			break;
		case 'b':
			bflag = 1;
			break;
		case 'c':
			// cvalue = state->optarg;
			break;
		case 'h':
			/* When getopt is active shell is not parsing
			 * command handler to print help message. It must
			 * be done explicitly.
			 */
			shell_help(sh);
			return SHELL_CMD_HELP_PRINTED;
		case '?':
			// if (state->optopt == 'c') {
			// 	shell_print(sh,
			// 		"Option -%c requires an argument.",
			// 		state->optopt);
			// } else if (isprint(state->optopt) != 0) {
			// 	shell_print(sh,
			// 		"Unknown option `-%c'.",
			// 		state->optopt);
			// } else {
			// 	shell_print(sh,
			// 		"Unknown option character `\\x%x'.",
			// 		state->optopt);
			// }
			return 1;
		default:
			break;
		}
	}

	shell_print(sh, "aflag = %d, bflag = %d", aflag, bflag);
	return 0;
}

static int cmd_demo_getopt(const struct shell *sh, size_t argc,
			      char **argv)
{
	char *cvalue = NULL;
	int aflag = 0;
	int bflag = 0;
	int c;

	while ((c = sys_getopt(argc, argv, "abhc:")) != -1) {
		switch (c) {
		case 'a':
			aflag = 1;
			break;
		case 'b':
			bflag = 1;
			break;
		case 'c':
			// cvalue = sys_getopt_optarg;
			break;
		case 'h':
			/* When getopt is active shell is not parsing
			 * command handler to print help message. It must
			 * be done explicitly.
			 */
			shell_help(sh);
			return SHELL_CMD_HELP_PRINTED;
		case '?':
			// if (sys_getopt_optopt == 'c') {
			// 	shell_print(sh,
			// 		"Option -%c requires an argument.",
			// 		sys_getopt_optopt);
			// } else if (isprint(sys_getopt_optopt) != 0) {
			// 	shell_print(sh, "Unknown option `-%c'.",
			// 		    sys_getopt_optopt);
			// } else {
			// 	shell_print(sh,
			// 		"Unknown option character `\\x%x'.",
			// 		sys_getopt_optopt);
			// }
			return 1;
		default:
			break;
		}
	}

	shell_print(sh, "aflag = %d, bflag = %d", aflag, bflag);
	return 0;
}
#endif

static int cmd_demo_params(const struct shell *sh, size_t argc, char **argv)
{
	shell_print(sh, "argc = %zd", argc);
	for (size_t cnt = 0; cnt < argc; cnt++) {
		shell_print(sh, "  argv[%zd] = %s", cnt, argv[cnt]);
	}

	return 0;
}

static int cmd_demo_hexdump(const struct shell *sh, size_t argc, char **argv)
{
	shell_print(sh, "argc = %zd", argc);
	for (size_t cnt = 0; cnt < argc; cnt++) {
		shell_print(sh, "argv[%zd]", cnt);
		shell_hexdump(sh, argv[cnt], strlen(argv[cnt]));
	}

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_demo,
	SHELL_CMD(hexdump, NULL, "Hexdump params command.", cmd_demo_hexdump),
	SHELL_CMD(params, NULL, "Print params command.", cmd_demo_params),
	SHELL_CMD(ping, NULL, "Ping command.", cmd_demo_ping),
	SHELL_CMD(board, NULL, "Show board name command.", cmd_demo_board),
#if defined CONFIG_SHELL_GETOPT
	SHELL_CMD(getopt_thread_safe, NULL,
		  "Cammand using getopt in thread safe way"
		  " looking for: \"abhc:\".",
		  cmd_demo_getopt_ts),
	SHELL_CMD(getopt, NULL, "Cammand using getopt in non thread safe way"
		  " looking for: \"abhc:\".\n", cmd_demo_getopt),
#endif
	SHELL_SUBCMD_SET_END /* Array terminated. */
);
SHELL_CMD_REGISTER(demo, &sub_demo, "Demo commands", NULL);

SHELL_CMD_ARG_REGISTER(version, NULL, "Show kernel version", cmd_version, 1, 0);

int main(void)
{
	char c;
	char count_str[11] = {0};
	const struct device *display_dev;
	lv_obj_t *hello_world_label;
	lv_obj_t *count_label;
	int ret;
 /***********This Section Is working**********/

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		// printf("Device not ready, aborting test");
		return 0;
	}
	k_sleep(K_MSEC(100));
	char keyVal = 0;
	printf("Hello World! %s\n", CONFIG_BOARD_TARGET);

	display_blanking_off(display_dev);
    printf("Display ready %d %d\n",lv_disp_get_hor_res(NULL),lv_disp_get_ver_res(NULL));
	// initShell();

	
    lv_obj_t *scr = lv_scr_act();
	terminal_init(scr);

/* Init Zephyr shell (UI backend) */
	// shell_lvgl_init();
	ret = startShell();
	printf("startShell %d\r\n",ret);
	k_sleep(K_MSEC(100));

	checkShellReady();

	 keyboard_init();
	 k_sleep(K_MSEC(100));

	 
 /**********This Section Is working***********/
    // if (!device_is_ready(uart)) {
    //     printk("UART not ready\n");
    //     return;
    // }

    // printk("UART RX test started\r\n");
    /* ---- TEST: auto-run help once ---- */
    // const char *cmd = "help\n";
    // for (int i = 0; cmd[i]; i++) {
    //     shell_feed_char(cmd[i]);
    //     k_sleep(K_MSEC(10));
    // }
			// printf("while started\n");
	while (1) 
	{
		// if ((count % 100) == 0U) {
		// 	sprintf(count_str, "%d", count/100U);
		// 	printf("%s\n",count_str);
		// }

		// lv_timer_handler();
		// ++count;
 /***********This Section Is working**********/

		lv_timer_handler();
		if (sb_available())
		{
			// printf("print 2\n");
    		keyboard_poll();
		}
		if(keyboard_key_available())
		{
			// printf("print 3\n");
			keyVal = keyboard_get_key();
			terminal_handle_key(keyVal);
			// shell_feed_char(keyVal);
			// on_key(keyVal);
			// printf("%c",keyVal);
		}
 /**********This Section Is working***********/
        // if (uart_poll_in(uart, &c) == 0) {
        //     printf("RX: %c (0x%02x)\r\n", c, c);
        // }
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
