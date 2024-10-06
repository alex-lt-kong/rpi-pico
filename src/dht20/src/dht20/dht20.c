// Code originally from:
// https://github.com/mikuhn/Raspberry-Pi-Pico-DHT20/tree/main DHT20 reference:
// https://files.seeedstudio.com/products/101020932/DHT20-Product-Manual.pdf

#include "dht20.h"

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

i2c_inst_t *dht20_device = i2c0;

int dht20_init() {
  uint8_t buf[] = {DHT20_COMMAND_STATUS};
  i2c_write_blocking(dht20_device, DHT20_ADDRESS, buf, 1, true);
  i2c_read_blocking(dht20_device, DHT20_ADDRESS, buf, 1, false);

  if (buf[0] != DHT20_RETURN_STATUS_OK) {
    printf("dht20_init() failed: 0x%x\n", buf[0]);
    return -1;
  }

  sleep_ms(10);
  return 0;
}

uint8_t crc8(uint8_t *ptr, uint8_t len) {
  uint8_t crc = 0xFF;
  while (len--) {
    crc ^= *ptr++;
    for (uint8_t i = 0; i < 8; i++) {
      if (crc & 0x80) {
        crc <<= 1;
        crc ^= 0x31;
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

int dht20_measure(dht20_reading *reading) {
  uint8_t buf[7] = {DHT20_TRIGGER_MEASUREMENT, DHT20_TRIGGER_MEASUREMENT_PARAM1,
                    DHT20_TRIGGER_MEASUREMENT_PARAM2};
  size_t len = 3;
  int ret;
  ret = i2c_write_blocking(dht20_device, DHT20_ADDRESS, buf, len, false);
  if (ret == PICO_ERROR_GENERIC || ret != len) {
    printf("i2c_write_blocking() failed to send data request\n");
    return -1;
  }

  len = 1;
  // Need to leave some time to client device
  sleep_ms(100);
  ret = i2c_read_blocking(dht20_device, DHT20_ADDRESS, buf, len, true);
  if (ret != len || ret == PICO_ERROR_GENERIC ||
      (buf[0] & DHT20_RETURN_STATUS_BUSY_MASK) != 0x0) {
    printf("DHT20 reading failed, ret: %d, buf[0]: 0x%x\n", ret, buf[0]);
    goto err_reading;
  }

  len = 7;
  ret = i2c_read_blocking(dht20_device, DHT20_ADDRESS, buf, len, false);
  if (ret == PICO_ERROR_GENERIC || ret != len) {
    printf("i2c_write_blocking() failed to send data request\n");
    goto err_reading;
  }

  uint8_t _crc = crc8(buf, 6);
  if (_crc != buf[6]) {
    printf("crc failed\n");
    goto err_reading;
  }
  // The conversion can also be found here:
  // https://github.com/RobTillaart/DHT20/blob/97ee6c789fdfa9d0dda0b6075f31cfbdafbd3d65/DHT20.cpp#L146-L172
  int humidity_raw = (buf[1] << 12) | (buf[2] << 8) | (buf[3] >> 4);
  int temperature_raw = ((buf[3] << 16) | (buf[4] << 8) | buf[5]) & 0xfffff;

  reading->humidity = (humidity_raw / powf(2, 20)) * 100;
  reading->temp_celsius = ((temperature_raw / powf(2, 20)) * 200) - 50;

  return 0;
err_reading:
  reading->humidity = FLT_MAX;
  reading->temp_celsius = FLT_MAX;
  return -1;
}