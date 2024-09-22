#include <stdio.h>
#include <string.h>

inline void convert_timestamp(unsigned long long timestamp_ms, char *output,
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

#define printf_ts(fmt, ...)                                                    \
  ({                                                                           \
    const size_t buff_size = 64;                                               \
    char datetime_buffer[buff_size];                                           \
    convert_timestamp(get_absolute_time() / 1000, datetime_buffer, buff_size); \
    printf("%s " fmt, datetime_buffer, ##__VA_ARGS__);                         \
  })
