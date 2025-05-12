#include <stdio.h>
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#define SCL 22
#define SDA 21
#define scl_speed 100000
#define MPU_ADDR 0x68
#define MPU_PWR 0x6B
#define port I2C_NUM_0

// #define Accel_con 0x1C
// #define Gyro_con 0x1B

i2c_master_bus_handle_t bus_handle;
i2c_master_dev_handle_t dev_handle;

void i2c_init()
{
    i2c_master_bus_config_t bus_con = {
        .scl_io_num = SCL,
        .sda_io_num = SDA,
        .i2c_port = port,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_con, &bus_handle));
}

void mpu_init()
{
    i2c_device_config_t dev_con = {
        .dev_addr_length = I2C_ADDR_BIT_7,
        .device_address = MPU_ADDR,
        .scl_speed_hz = scl_speed,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_con, &dev_handle));
}

void mpu_wake()
{
    uint8_t data[2] = {MPU_PWR, 0x00};
    // uint8_t acc_con[2] = {Accel_con, 0x00};
    // uint8_t gyro_scale[2] = {Gyro_con, 0x00};

    i2c_master_transmit_multi_buffer_info_t buffer_info[1] = {
        {.write_buffer = data, .buffer_size = sizeof(data)},
        // {.write_buffer = gyro_scale, .buffer_size = sizeof(gyro_scale)},
    };

    ESP_ERROR_CHECK(i2c_master_multi_buffer_transmit(dev_handle, buffer_info, sizeof(buffer_info) / sizeof(i2c_master_transmit_multi_buffer_info_t), -1));
}

void read_accel()
{
    uint8_t reg = 0x3B;
    uint8_t data[14] = {0};

    while (1)
    {
        ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, &reg, 1, data, 14, -1));

        int16_t ax = (data[0] << 8) | data[1];
        int16_t ay = (data[2] << 8) | data[3];
        int16_t az = (data[4] << 8) | data[5];
        int16_t gx = (data[8] << 8) | data[9];
        int16_t gy = (data[10] << 8) | data[11];
        int16_t gz = (data[12] << 8) | data[13];

        float aRes = 2.0 / 32768.0;
        float gRes = 250.0 / 32768.0;

        printf("Acc X = %.3f | Acc Y = %.3f | Acc Z = %.3f | Gyro X = %.3f | Gyro Y = %.3f | Gyro Z = %.3f\n", ax * aRes, ay * aRes, az * aRes, gx * gRes, gy * gRes, gz * gRes);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    printf("Initializing I2C Master....\n");
    i2c_init();

    printf("Initializing I2C Slave....\n");
    mpu_init();

    vTaskDelay(pdMS_TO_TICKS(100));

    printf("Waking up MPU....\n");
    mpu_wake();

    vTaskDelay(pdMS_TO_TICKS(100));

    read_accel();
}