
#include <stddef.h>

void convert_timestamp(unsigned long long timestamp_ms, char *output,
                       size_t output_size);

#define printf_ts(fmt, ...)                                                    \
  ({                                                                           \
    const size_t buff_size = 64;                                               \
    char datetime_buffer[buff_size];                                           \
    convert_timestamp(get_absolute_time() / 1000, datetime_buffer, buff_size); \
    printf("%s " fmt, datetime_buffer, ##__VA_ARGS__);                         \
  })

void sleep_with_output(int delay_sec, const char *reason);
