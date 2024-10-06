// Originally https://github.com/mikuhn/Raspberry-Pi-Pico-DHT20/tree/main

#include "dht20.h"

#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#include <stdio.h>

// PIN number is in GPIO, seems we can't rename these two define's
#define PICO_DEFAULT_I2C_SDA_PIN 2
#define PICO_DEFAULT_I2C_SCL_PIN 3

int main() {
  (void)stdio_init_all();
  // Need to wait for a while; otherwise some early output would be missed
  sleep_ms(5000);
  printf("DHT20-driver started\n");
  dht20_device = i2c1;
  unsigned int buadrate = i2c_init(dht20_device, 100 * 1000);
  printf("buadrate of I2C protocol: %u\n", buadrate);
  gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
  // Make the I2C pins available to picotool
  bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN,
                             GPIO_FUNC_I2C));

  if (dht20_init() != 0) {
    printf("dht20_init() failed\n");
    return -1;
  }

  dht20_reading dht20_result;

  printf("humidity temperature\n");

  while (1) {
    dht20_measure(&dht20_result);
    printf("%.02f%%   %.02f°C\n", dht20_result.humidity,
           dht20_result.temp_celsius);
    sleep_ms(2000);
  }
  return 0;
}