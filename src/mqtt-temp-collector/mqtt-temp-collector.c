#include "crypto_consts.h"
#include "dht20/src/dht20/dht20.h"
#include "helper.h"

#include "hardware/i2c.h"
#include "lwip/altcp_tls.h"
#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "pico/binary_info.h"
#include "pico/cyw43_arch.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#include <math.h>
#include <stdio.h>

dht20_reading dht20_result;

// PIN number as GPIO
#define PICO_DEFAULT_I2C_SDA_PIN 0
#define PICO_DEFAULT_I2C_SCL_PIN 1

#define MQTT_TLS 0
#define PAYLOAD_SIZE 128
#define MQTT_KEEPALIVE_SEC 30

bool dnsLookupInProgress = false;
ip_addr_t ServerIP;

typedef void (*Callback_type)(const u8_t *Data, u16_t Len);

void mqtt_publish_cb(void *Arg, err_t Result) {
  if (Result != ERR_OK) {
    printf("error: %d\n", Result);
  } else {
    printf("done\n");
  }
}

void dns_found_cb(const char *Hostname, const ip_addr_t *Ipaddr,
                  void *Arg) // DNS lookup callback
{
  dnsLookupInProgress = false;
  if (Ipaddr) {
    printf_ts("Found address: %s\n", ipaddr_ntoa(Ipaddr));
    ServerIP = *Ipaddr;
  } else {
    printf_ts("DNS lookup failed\n");
  }
}

char *intToChar(int Number) // Converting int to char
{
  static char ReturnChar[8] = ""; // 7 digits + null terminator max
  itoa(Number, ReturnChar, 10);
  return ReturnChar;
}

void mqtt_incoming_publish_cb(void *Arg, const char *Topic, u32_t Tot_len) {
  printf_ts("Incoming topic: %s ,total length: %u\n", Topic,
            (unsigned int)Tot_len);
}

void mqtt_incoming_data_cb(void *Arg, const u8_t *Data, u16_t Len, u8_t Flags) {
  printf_ts("Incoming payload with length %d, flags %u\n", Len,
            (unsigned int)Flags);

  // Last fragment of payload received (or the whole payload fits receive
  // buffer (MQTT_VAR_HEADER_BUFFER_LEN, MQTT_DATA_FLAG_LAST)
  if (Flags == 1) {
    ((Callback_type)Arg)(Data, Len);
  } else {
    printf_ts(
        "Max payload exceeded"); /// TODO: Implement multiple package handling
  }
}

void mqtt_connection_cb(mqtt_client_t *Client, void *Arg,
                        mqtt_connection_status_t Status) {
  if (Status == MQTT_CONNECT_ACCEPTED) {
    printf_ts("mqtt_connection_cb(): MQTT_CONNECT_ACCEPTED\n");
  } else if (Status == MQTT_CONNECT_DISCONNECTED) {
    printf_ts("mqtt_connection_cb(): MQTT_CONNECT_DISCONNECTED\n");
  } else if (Status == MQTT_CONNECT_REFUSED_NOT_AUTHORIZED_) {
    printf_ts("mqtt_connection_cb(): MQTT_CONNECT_REFUSED_NOT_AUTHORIZED_\n");
  } else {
    printf_ts("mqtt_connection_cb() error, status code: %d\n", Status);
  }
}

int initialize_wifi() {
  printf_ts("Initializing WiFi\n");

  int rc;

  cyw43_arch_enable_sta_mode();
  printf_ts("Connecting to Hotspot: [%s]...\n", WIFI_SSID);
  if ((rc = cyw43_arch_wifi_connect_timeout_ms(
           WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) != 0) {
    printf_ts("cyw43_arch_wifi_connect_timeout_ms() failed, rc: %d\n", rc);
    return -1;
  }
  printf_ts("wifi connected\n");
  return 0;
}

int init_server_ip() {
  if (MQTT_BROKER_HOSTNAME != NULL && strlen(MQTT_BROKER_HOSTNAME) > 0) {
    printf_ts("Looking up IP for %s...\n", MQTT_BROKER_HOSTNAME);
    err_t err =
        dns_gethostbyname(MQTT_BROKER_HOSTNAME, &ServerIP, dns_found_cb, NULL);
    if (err == ERR_OK) // DNS name found in cache
    {
      printf_ts("Found cached address\n");
      dnsLookupInProgress = false;
    } else if (err == ERR_INPROGRESS) {
      printf_ts("DNS request enqueued, waiting to be resolved\n");
      dnsLookupInProgress = true;
    } else {
      printf_ts("dns_gethostbyname() failed: ERR_ARG");
      dnsLookupInProgress = false;
    }
    absolute_time_t t0 = get_absolute_time();
    while (dnsLookupInProgress) // Waiting for the DNS lookup to finish
    {
      sleep_ms(100);
      if (absolute_time_diff_us(t0, get_absolute_time()) > 10000000) {
        printf_ts("DNS lookup timeout\n");
        return -1;
      }
    }
  } else {
    // DNS not specified, use MQTTServerIP instead
    ip4addr_aton(MQTT_BROKER_IP, &ServerIP);
  }
  return 0;
}

int init_dht20() {
  i2c_init(dht20_device, 100 * 1000);
  gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
  // Make the I2C pins available to picotool
  bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN,
                             GPIO_FUNC_I2C));

  return dht20_init();
}

int main() {
  int publish_interval_sec = 60;
  int delay_sec = 10;
  int dht20_ret;
  mqtt_client_t *mc;
  bool led_on = true;
  absolute_time_t t0 = 0;
  struct mqtt_connect_client_info_t ci;
  err_t mqtt_err;

  stdio_init_all();

  sleep_with_output(delay_sec, "initializing basic components");
  if (cyw43_arch_init() != 0) {
    printf_ts("cyw43_arch_init() failed, wifi not initialized\n");
    goto err_cyw43_arch_init_failed;
  }
  printf_ts("cyw43_driver and lwIP stack initialized\n");
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);

  mc = mqtt_client_new();
  if (mc == NULL) {
    printf_ts("mqtt_client_new() failed\n");
    goto err_mqtt_client_new_failed;
  }
  printf_ts("mqtt client initialized\n");

  if ((dht20_ret = init_dht20())) {
    printf_ts("init_dht20() failed\n");
    goto err_dht20_failed;
  }
  printf_ts("DHT20 initialized\n");

  while (1) {
    // This is also for wifi disconnect retry etc
    sleep_with_output(delay_sec, "this event loop iteration");

    int rc;
    if ((rc = initialize_wifi()) != 0) {
      printf_ts("initialize_wifi() failed, rc: %d\n", rc);
      goto err_wifi_initialization_failed;
    }

    if ((rc = init_server_ip()) != 0) {
      printf_ts("init_server_ip() failed, rc: %d", rc);
      goto err_init_server_ip_failed;
    }

    memset(&ci, 0, sizeof(ci));
    ci.client_id = MQTT_CLIENT_ID;
    ci.client_user = MQTT_USERNAME;
    ci.client_pass = MQTT_PASSWORD;
    ci.keep_alive = MQTT_KEEPALIVE_SEC;
    ci.will_topic = NULL;
    ci.will_msg = NULL;
    ci.will_qos = 0;

#if MQTT_TLS

    struct altcp_tls_config *tls_config;
    const char *cert = CRYPTO_CERT;
    printf_ts("Setting up TLS with cert.\n");

    // tls_config = altcp_tls_create_config_client((const u8_t *)cert,
    // 1 + strlen((const char *)cert));

    // TLS validation turned off...
    tls_config = altcp_tls_create_config_client(NULL, 0);

    if (tls_config == NULL) {
      printf_ts("Failed to initialize "
                "TLS config\n");
      goto err_tls_failed;
    } else {
      printf_ts("TLS configured\n");
    }

    // mbedtls_ssl_conf_authmode(&(tls_config->conf),
    // MBEDTLS_SSL_VERIFY_REQUIRED);

    ci.tls_config = tls_config;
#endif

    cyw43_arch_lwip_begin();

    mqtt_set_inpub_callback(mc, mqtt_incoming_publish_cb, mqtt_incoming_data_cb,
                            LWIP_CONST_CAST(void *, &mc));
    mqtt_err = mqtt_client_connect(mc, &ServerIP, atoi(MQTT_BROKER_PORT),
                                   mqtt_connection_cb,
                                   LWIP_CONST_CAST(void *, &ci), &ci);
    cyw43_arch_lwip_end();
    if (mqtt_err != ERR_OK && mqtt_err != ERR_ISCONN) {
      printf_ts("mqtt_client_connect() "
                "failed: %d\n",
                mqtt_err);
      goto err_mqtt_client_connect_failed;
    }

    // Waiting for the MQTT
    // connection to establish
    // sleep_ms(delay_sec * 1000);
    sleep_with_output(10, "checking MQTT "
                          "connection status");

    if (mqtt_client_is_connected(mc) == 0) {
      printf_ts("mqtt NOT connected\n");
      goto err_mqtt_client_connect_failed;
    } else {
      printf_ts("mqtt connected\n");
    }

    printf_ts("Entering publishing loop, "
              "interval: %d sec\n",
              publish_interval_sec);

    while (1) {
      sleep_ms(1000);
      if (absolute_time_diff_us(t0, get_absolute_time()) <
          publish_interval_sec * 1000 * 1000) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
        led_on = !led_on;
        continue;
      }

      cyw43_arch_poll();
      char payload[PAYLOAD_SIZE];
      if ((rc = dht20_measure(&dht20_result)) != 0) {
        printf_ts("dht20_measure() "
                  "failed: %d\n",
                  rc);
        continue;
      }
      snprintf(payload, PAYLOAD_SIZE, "{\"temp_celsius\": %.01f, \"RH\": %.0f}",
               dht20_result.temp_celsius, round(dht20_result.humidity));
      printf_ts("payload: %s\n", payload);
      cyw43_arch_lwip_begin();
      printf_ts("Publishing data to " MQTT_TOPIC "...");
      if ((mqtt_err = mqtt_publish(mc, MQTT_TOPIC, payload, strlen(payload), 1,
                                   0, mqtt_publish_cb, (void *)&MQTT_TOPIC)) !=
          ERR_OK) {
        printf_ts("mqtt_publish() failed: %d\n", mqtt_err);
        cyw43_arch_lwip_end();
        goto err_mqtt_publish_failed;
      }
      cyw43_arch_lwip_end();
      t0 = get_absolute_time();
    }
  err_mqtt_publish_failed:
    mqtt_disconnect(mc);
  err_mqtt_client_connect_failed:
  err_init_server_ip_failed:
  err_wifi_initialization_failed:
    continue;
  }
  cyw43_arch_deinit();
err_dht20_failed:
err_mqtt_client_new_failed:
err_cyw43_arch_init_failed:
  return 0;
}
