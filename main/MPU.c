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

void calibrate(int32_t *offset_arr, int size)
{
    int32_t Ax_sum = 0, Ay_sum = 0, Az_sum = 0, Gx_sum = 0, Gy_sum = 0, Gz_sum = 0;
    int samples = 1000;
    int i = 0;
    uint8_t reg = 0x3B;
    uint8_t data[14] = {0};
    printf("\nCalibrating...\n");
    while (i < 1000)
    {
        ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, &reg, 1, data, 14, -1));

        int16_t ax = (data[0] << 8) | data[1];
        Ax_sum += ax;
        int16_t ay = (data[2] << 8) | data[3];
        Ay_sum += ay;
        int16_t az = (data[4] << 8) | data[5];
        Az_sum += az;
        int16_t gx = (data[8] << 8) | data[9];
        Gx_sum += gx;
        int16_t gy = (data[10] << 8) | data[11];
        Gy_sum += gy;
        int16_t gz = (data[12] << 8) | data[13];
        Gz_sum += gz;

        i++;
        // printf("%d\n", i);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    int16_t Ax_offset = Ax_sum / samples;
    offset_arr[0] = Ax_offset;

    int16_t Ay_offset = Ay_sum / samples;
    offset_arr[1] = Ay_offset;

    int16_t Az_offset = Az_sum / samples;
    offset_arr[2] = Az_offset - 16384;

    int16_t Gx_offset = Gx_sum / samples;
    offset_arr[3] = Gx_offset;

    int16_t Gy_offset = Gy_sum / samples;
    offset_arr[4] = Gy_offset;

    int16_t Gz_offset = Gz_sum / samples;
    offset_arr[5] = Gz_offset;

    printf("Ax off = %d |Ay off = %d |Az off = %d |Gx off = %d |Gy off = %d |Gz off = %d \n", Ax_offset, Ay_offset, Az_offset, Gx_offset, Gy_offset, Gz_offset);
}

void read_accel_gyro()
{
    uint8_t reg = 0x3B;
    uint8_t data[14] = {0};

    float aRes = 2.0 / 32768.0;
    float gRes = 250.0 / 32768.0;

    int32_t offset_arr[6];
    calibrate(offset_arr, 6);

    while (1)
    {
        ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, &reg, 1, data, 14, -1));

        int16_t ax = ((data[0] << 8) | data[1]) - offset_arr[0];
        int16_t ay = ((data[2] << 8) | data[3]) - offset_arr[1];
        int16_t az = ((data[4] << 8) | data[5]) - offset_arr[2];
        int16_t gx = ((data[8] << 8) | data[9]) - offset_arr[3];
        int16_t gy = ((data[10] << 8) | data[11]) - offset_arr[4];
        int16_t gz = ((data[12] << 8) | data[13]) - offset_arr[5];

        printf("Acc X = %.3f | Acc Y = %.3f | Acc Z = %.3f | Gyro X = %.3f | Gyro Y = %.3f | Gyro Z = %.3f\n", (ax * aRes) * 9.8, (ay * aRes) * 9.8, (az * aRes) * 9.8, gx * gRes, gy * gRes, gz * gRes);

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

    read_accel_gyro();
}