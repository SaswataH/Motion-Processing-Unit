#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"

#define port I2C_NUM_0
#define SDA 21
#define SCL 22
#define scl_speed 400000
#define MPU_ADDR 0x68
#define MPU_PWR 0x6B

#define GYRO_CON 0x1B
#define ACC_CON 0x1C

i2c_master_bus_handle_t bus_handle;
i2c_master_dev_handle_t dev_handle;

#define Acc_Out_H 0x3B

static const char *TAG = "MPU";

void i2c_init()
{
    // configure the master bus
    i2c_master_bus_config_t bus_conf = {
        .i2c_port = port,
        .sda_io_num = SDA,
        .scl_io_num = SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    // allocate and initialize the master bus
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_conf, &bus_handle));
}

void mpu_init()
{
    i2c_device_config_t dev_conf = {
        .dev_addr_length = I2C_ADDR_BIT_7,
        .device_address = MPU_ADDR,
        .scl_speed_hz = scl_speed,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_conf, &dev_handle));
}

void wake_mpu()
{
    uint8_t data_buffer[2] = {MPU_PWR, 0x00};
    uint8_t acc_scale[2] = {ACC_CON, 0x00};
    uint8_t gyro_scale[2] = {GYRO_CON, 0x00};

    // array of buffer info
    i2c_master_transmit_multi_buffer_info_t buffer_info[3] = {
        {.write_buffer = data_buffer, .buffer_size = sizeof(data_buffer)},
        {.write_buffer = acc_scale, .buffer_size = sizeof(acc_scale)},
        {.write_buffer = gyro_scale, .buffer_size = sizeof(gyro_scale)},
    };

    ESP_ERROR_CHECK(i2c_master_multi_buffer_transmit(dev_handle, buffer_info, sizeof(buffer_info) / sizeof(i2c_master_transmit_multi_buffer_info_t), -1));
}

void read_accel()
{
    uint8_t reg = 0x3B; // ACCEL_XOUT_H — first of 14 sequential registers
    uint8_t data[6] = {0};

    while (1)
    {
        esp_err_t err = i2c_master_transmit_receive(dev_handle, &reg, 1, data, 6, -1);
        // Decode and print accel values
        int16_t ax = (data[0] << 8) | data[1];
        int16_t ay = (data[2] << 8) | data[3];
        int16_t az = (data[4] << 8) | data[5];

        float aRes = 2.0 / 32768.0;
        printf("Accel Gs: X=%.3f | Y=%.3f | Z=%.3f\n", ax * aRes, ay * aRes, az * aRes);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing I2C Master....");
    i2c_init();

    ESP_LOGI(TAG, "Initializing I2C Slave(MPU)....");
    mpu_init();

    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "Waking up I2C Slave(MPU)....");
    wake_mpu();

    vTaskDelay(pdMS_TO_TICKS(100));

    read_accel();
}
