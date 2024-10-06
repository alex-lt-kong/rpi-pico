/**
 * Copyright (c) 2023 Michael Kuhn
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DHT20_H
#define DHT20_H

#include "hardware/i2c.h"

#define DHT20_ADDRESS 0x38
#define DHT20_COMMAND_STATUS 0x71
#define DHT20_RETURN_STATUS_OK 0x18
#define DHT20_TRIGGER_MEASUREMENT 0xAC
#define DHT20_TRIGGER_MEASUREMENT_PARAM1 0x33
#define DHT20_TRIGGER_MEASUREMENT_PARAM2 0x00
#define DHT20_RETURN_STATUS_BUSY_MASK 0x80

typedef struct {
  float humidity;
  float temp_celsius;
} dht20_reading;

int dht20_init();
int dht20_measure(dht20_reading *measurement);

// Specify I2C device, values are typically i2c0 or i2c1 for a Pico W board,
// default to i2c0
extern i2c_inst_t *dht20_device;

#endif // DHT20_H