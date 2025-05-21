# Motion-Processing-Unit

## Description

This is a test of a Motion Processing Unit(MPU) - "MPU 6050" module using an ESP-32 microcontroller.
The current code uses the 3-axis (X , Y , Z) Accelerometer to measure the total acceleration (Linear + Static) on the sensor and 3-axis (x , Y , Z) Gyroscope to measure the total angular velocity.

When the MPU module is stationary, the accelerometer will read approx. ±1g along the axis aligned with gravity. The remaining axes will provide a value close to ±0g. This mechanism aids us estimating the orientation of the sensor when it's stationary.

The ESP-32 microcontroller uses I2C protocol to read the raw values from the dedicated Accelerometer registers of the MPU 6050(as provided in the official datasheets). The raw 16-bit values are then converted to physical unit(g - in this case). The resultant values oscillate in the range of -1g to +1g approximately.

## Hardware and Software used:

1. ESP-32 (ESP-WROOM-32)
2. MPU 6050
3. Jumper Wires (for basic connections)
4. 4.7k ohm pull up resistors
5. Breadboard
6. ESP-IDF (SDK)

## References

[ESP-32 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf)

[ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html)

[MPU 6050 Datasheet](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)

[MPU 6050 Register Map](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf)

## Present Problems

1. Applying a suitable filter algorithms to stabilise short-term Gyroscope values and Long-term Accelerometer values. (20/5/2025)

## Further Implementation

1. Using the Accelerometer and Gyroscope values to measure the angles in which the sensor is oriented.
