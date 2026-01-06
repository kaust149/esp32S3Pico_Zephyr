//
//  "SouthBridge" functions
//
//  The PicoCalc on-board processor acts as a "southbridge", managing lower-speed functions
//  that provides access to the keyboard, battery, and other peripherals.
//

#include <stdatomic.h>
#include <stddef.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

#include "southbridge.h"

#define I2C_DEV_NODE DT_NODELABEL(i2c1)

const struct device *i2c_dev = DEVICE_DT_GET(I2C_DEV_NODE);
static bool sb_initialised = false;
volatile atomic_bool sb_i2c_in_use = false; // flag to indicate if I2C bus is in use

uint8_t buffer[10];
static char last_byte;
/*
 * @brief Callback which is called when a write request is received from the master.
 * @param config Pointer to the target configuration.
 */
int sample_target_write_requested_cb(struct i2c_target_config *config)
{
	printk("sample target write requested\n");
	return 0;
}

/*
 * @brief Callback which is called when a write is received from the master.
 * @param config Pointer to the target configuration.
 * @param val The byte received from the master.
 */
int sample_target_write_received_cb(struct i2c_target_config *config, uint8_t val)
{
	printk("sample target write received: 0x%02x\n", val);
	last_byte = val;
	return 0;
}

/*
 * @brief Callback which is called when a read request is received from the master.
 * @param config Pointer to the target configuration.
 * @param val Pointer to the byte to be sent to the master.
 */
int sample_target_read_requested_cb(struct i2c_target_config *config, uint8_t *val)
{
	printk("sample target read request: 0x%02x\n", *val);
	*val = 0x42;
	return 0;
}

/*
 * @brief Callback which is called when a read is processed from the master.
 * @param config Pointer to the target configuration.
 * @param val Pointer to the next byte to be sent to the master.
 */
int sample_target_read_processed_cb(struct i2c_target_config *config, uint8_t *val)
{
	printk("sample target read processed: 0x%02x\n", *val);
	*val = 0x43;
	return 0;
}

/*
 * @brief Callback which is called when the master sends a stop condition.
 * @param config Pointer to the target configuration.
 */
int sample_target_stop_cb(struct i2c_target_config *config)
{
	printk("sample target stop callback\n");
	return 0;
}

static struct i2c_target_callbacks sample_target_callbacks = {
	.write_requested = sample_target_write_requested_cb,
	.write_received = sample_target_write_received_cb,
	.read_requested = sample_target_read_requested_cb,
	.read_processed = sample_target_read_processed_cb,
	.stop = sample_target_stop_cb,
};

//
//  Protect access to the "South Bridge"
//

//  Is the southbridge available?
bool sb_available()
{
    return atomic_load(&sb_i2c_in_use) == false;
}

static size_t sb_write(uint8_t *src, size_t len)
{
    int result = 0;
    // int result = i2c_write_dt(i2c_dev,src, len);

	struct i2c_msg msg;

	msg.buf = (uint8_t *)src;
	msg.len = len;
	msg.flags = I2C_MSG_WRITE | I2C_MSG_STOP;

	result = i2c_transfer(i2c_dev, &msg, 1, SB_ADDR);
    if (result == 0)
    {
        // Write error
        return len;
    }
    else
    {
        return 0;
    }
    // int result = i2c_write_timeout_us(SB_I2C, SB_ADDR, src, len, false, SB_I2C_TIMEOUT_US * len);
    // if (result == PICO_ERROR_GENERIC || result == PICO_ERROR_TIMEOUT)
    // {
    //     // Write error
    //     return 0;
    // }
    // return result;
}

static size_t sb_read(uint8_t *dst, size_t len)
{
    int result = 0;//i2c_read_dt(i2c_dev,dst, len);

    struct i2c_msg msg;

	msg.buf = dst;
	msg.len = len;
	msg.flags = I2C_MSG_READ | I2C_MSG_STOP;

	result = i2c_transfer(i2c_dev, &msg, 1, SB_ADDR);

    if (result == 0)
    {
        // Write error
        return len;
    }
    else
    {
        return 0;
    }
    // int result = i2c_read_timeout_us(SB_I2C, SB_ADDR, dst, len, false, SB_I2C_TIMEOUT_US * len);
    // if (result == PICO_ERROR_GENERIC || result == PICO_ERROR_TIMEOUT)
    // {
    //     // Read error
    //     return 0;
    // }
    // return result;
}

// Read the keyboard
uint16_t sb_read_keyboard()
{
    atomic_store(&sb_i2c_in_use, true);

    buffer[0] = SB_REG_FIF;             // command to check if key is available
    if (sb_write(buffer, 1) != 1)
    {
        atomic_store(&sb_i2c_in_use, false);
        return 0;
    }
    if (sb_read(buffer, 2) != 2)
    {
        atomic_store(&sb_i2c_in_use, false);
        return 0;
    }
    atomic_store(&sb_i2c_in_use, false);

    return buffer[0] << 8 | buffer[1];
}

uint16_t sb_read_keyboard_state()
{
    uint8_t buffer[2];

    atomic_store(&sb_i2c_in_use, true);
    buffer[0] = SB_REG_KEY;             // command to read key state
    if (sb_write(buffer, 1) != 1)
    {
        atomic_store(&sb_i2c_in_use, false);
        return 0;
    }
    if (sb_read(buffer, 2) != 2)
    {
        atomic_store(&sb_i2c_in_use, false);
        return 0;
    }
    atomic_store(&sb_i2c_in_use, false);

    return buffer[0];
}

// Read the battery level from the southbridge
uint8_t sb_read_battery()
{
    uint8_t buffer[2];

    atomic_store(&sb_i2c_in_use, true);
    buffer[0] = SB_REG_BAT; // command to read battery level
    if (sb_write(buffer, 1) != 1)
    {
        atomic_store(&sb_i2c_in_use, false);
        return 0;
    }
    if (sb_read(buffer, 2) != 2)
    {
        atomic_store(&sb_i2c_in_use, false);
        return 0;
    }
    atomic_store(&sb_i2c_in_use, false);

    return buffer[1];
}

// Read the LCD backlight level
uint8_t sb_read_lcd_backlight()
{
    uint8_t buffer[2];

    atomic_store(&sb_i2c_in_use, true);
    buffer[0] = SB_REG_BKL; // command to read LCD backlight
    if (sb_write(buffer, 1) != 1)
    {
        atomic_store(&sb_i2c_in_use, false);
        return 0;
    }
    if (sb_read(buffer, 2) != 2)
    {
        atomic_store(&sb_i2c_in_use, false);
        return 0;
    }
    atomic_store(&sb_i2c_in_use, false);

    return buffer[1];
}

// Write the LCD backlight level
uint8_t sb_write_lcd_backlight(uint8_t brightness)
{
    uint8_t buffer[2];

    atomic_store(&sb_i2c_in_use, true);
    buffer[0] = SB_REG_BKL | SB_WRITE; // command to write LCD backlight
    buffer[1] = brightness;
    if (sb_write(buffer, 2) != 2)
    {
        atomic_store(&sb_i2c_in_use, false);
        return 0;
    }
    if (sb_read(buffer, 2) != 2)
    {
        atomic_store(&sb_i2c_in_use, false);
        return 0;
    }
    atomic_store(&sb_i2c_in_use, false);

    return buffer[1];
}

// Read the keyboard backlight level
uint8_t sb_read_keyboard_backlight()
{
    uint8_t buffer[2];

    atomic_store(&sb_i2c_in_use, true);
    buffer[0] = SB_REG_BK2; // command to read keyboard backlight
    if (sb_write(buffer, 1) != 1)
    {
        atomic_store(&sb_i2c_in_use, false);
        return 0;
    }
    if (sb_read(buffer, 2) != 2)
    {
        atomic_store(&sb_i2c_in_use, false);
        return 0;
    }
    atomic_store(&sb_i2c_in_use, false);

    return buffer[1];
}

// Write the keyboard backlight level
uint8_t sb_write_keyboard_backlight(uint8_t brightness)
{
    uint8_t buffer[2];

    atomic_store(&sb_i2c_in_use, true);
    buffer[0] = SB_REG_BK2 | SB_WRITE; // command to write keyboard backlight
    buffer[1] = brightness;
    if (sb_write(buffer, 2) != 2)
    {
        atomic_store(&sb_i2c_in_use, false);
        return 0;
    }
    if (sb_read(buffer, 2) != 2)
    {
        atomic_store(&sb_i2c_in_use, false);
        return 0;
    }
    atomic_store(&sb_i2c_in_use, false);

    return buffer[1];
}

bool sb_is_power_off_supported()
{
    uint8_t buffer[2];

    atomic_store(&sb_i2c_in_use, true);
    buffer[0] = SB_REG_OFF; // read the power-off register
    if (sb_write(buffer, 1) != 1)
    {
        atomic_store(&sb_i2c_in_use, false);
        return false;
    }
    if (sb_read(buffer, 2) != 2)
    {
        atomic_store(&sb_i2c_in_use, false);
        return false;
    }
    atomic_store(&sb_i2c_in_use, false);

    return buffer[1] > 0;
}

bool sb_write_power_off_delay(uint8_t delay_seconds)
{
    uint8_t buffer[2];

    atomic_store(&sb_i2c_in_use, true);
    buffer[0] = SB_REG_OFF | SB_WRITE; // command to write power-off delay
    buffer[1] = delay_seconds;
    if (sb_write(buffer, 2) != 2)
    {
        atomic_store(&sb_i2c_in_use, false);
        return false;
    }
    atomic_store(&sb_i2c_in_use, false);
    return true;
}

bool sb_reset(uint8_t delay_seconds)
{
    uint8_t buffer[2];

    atomic_store(&sb_i2c_in_use, true);
    buffer[0] = SB_REG_RST | SB_WRITE; // command to reset the PicoCalc
    buffer[1] = delay_seconds;
    if (sb_write(buffer, 2) != 2)
    {
        atomic_store(&sb_i2c_in_use, false);
        return false;
    }
    if (sb_read(buffer, 2) != 2)
    {
        atomic_store(&sb_i2c_in_use, false);
        return false;
    }
    atomic_store(&sb_i2c_in_use, false);
    return true;
}

// Initialize the southbridge
void sb_init()
{
    if (sb_initialised)
    {
        return; // already initialized
    }
    if (!device_is_ready(i2c_dev)) {
        printf("I2C device %s is not ready!", i2c_dev->name);
        return;
    }
    // printf("I2C device %s is ready.", i2c_dev->name);

    // 	struct i2c_target_config target_cfg = {
	// 	.address = 0x60,
	// 	.callbacks = &sample_target_callbacks,
	// };

	// printk("i2c custom target sample\n");

	// if (i2c_target_driver_register(i2c_dev) < 0) {
	// 	printk("Failed to register target\n");
	// 	return -1;
	// }

    // i2c_init(SB_I2C, SB_BAUDRATE);
    // gpio_set_function(SB_SCL, GPIO_FUNC_I2C);
    // gpio_set_function(SB_SDA, GPIO_FUNC_I2C);
    // gpio_pull_up(SB_SCL);
    // gpio_pull_up(SB_SDA);

    // Set the initialised flag
    sb_initialised = true;
}
