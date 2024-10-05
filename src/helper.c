#include "helper.h"
#include "pico/time.h"

#include <stdio.h>
#include <string.h>

void convert_timestamp(unsigned long long timestamp_ms, char *output,
                       size_t output_size) {
  unsigned long long days, hours, minutes, seconds, milliseconds;

  days = timestamp_ms / (24 * 60 * 60 * 1000);
  timestamp_ms %= (24 * 60 * 60 * 1000);

  hours = timestamp_ms / (60 * 60 * 1000);
  timestamp_ms %= (60 * 60 * 1000);

  minutes = timestamp_ms / (60 * 1000);
  timestamp_ms %= (60 * 1000);

  seconds = timestamp_ms / 1000;
  timestamp_ms %= 1000;

  milliseconds = timestamp_ms;

  snprintf(output, output_size, "[%llu.%02llu:%02llu:%02llu.%03llu]", days,
           hours, minutes, seconds, milliseconds);
}

void sleep_with_output(int delay_sec, const char *reason) {
  for (int i = 0; i < delay_sec; ++i) {
    printf_ts("Waiting for %d sec before [%s]...\n", delay_sec - i, reason);
    sleep_ms(1000);
  }
}