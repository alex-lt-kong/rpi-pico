// Modeled after this project:
// https://github.com/sampsapenna/dht20-pico-example/tree/master

#include <stdio.h>
#include <string.h>

#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

#include "DHT20.h"

#define I2C_SDA 0
#define I2C_SCL 1
#define EXAMPLE_I2C_INST i2c0

DHT20 sens;
DHT20 *sens_ptr = &sens;

int setup() {
  // Set up stdio
  stdio_init_all();
#ifndef DHT20_EXAMPLE_SKIP_INIT_SLEEP
  sleep_ms(2000);
#endif
  printf("Running controller setup.\n");
  printf("Setting up i2c\n");
  // Setup i2c @ 100k
  i2c_init(EXAMPLE_I2C_INST, 100000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);
  // Make the I2C pins available to picotool
  bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));
  printf("Set up i2c.\n");

  // Set up temp and humid sensor
  printf("Initialize DHT20.\n");
  int sensor_ret = DHT20_init(sens_ptr);
  if (sensor_ret != DHT20_OK) {
    printf("Failed to initialize the sensor.\n");
    printf("Sensor return value %d\n", sensor_ret);
    return sensor_ret;
  }
  printf("Initialized DHT20.\n");

  // Sleep for 10ms after successful sensor initialization
  sleep_ms(10);
  return 0;
}

int loop() {
  int ret = 0;
  uint32_t count = 1;
  printf("Starting the temperature, humidity fetch loop.\n");
  while (true) {
    ret = getMeasurement(sens_ptr);
    if (ret != DHT20_OK) {
      printf("Measurement %d failed with error value %d\n", count, ret);
      printf("Trying again after 10s...\n");
    } else {
      printf("Measurements: \n");
      printf("--- Temperature: %5.2f C°", getTemperature(sens_ptr));
      printf("--- Humidity: %5.2f \%RH\n", getHumidity(sens_ptr));
    }
    count++;
    sleep_ms(10000);
  }

  return 0;
}

int main() {
  int ret;
  ret = setup();
  // In case we add failure conditions after setup, handle them after
  // label "failover"
  if (ret)
    goto failover;
  return loop();
failover:
  printf("Failed to start loop.\n");
  return ret;
}