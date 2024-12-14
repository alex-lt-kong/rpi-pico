// https://github.com/cniles/picow-iot/blob/main/picow_iot.c
#include "helper.h"

#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "pico/cyw43_arch.h"
#include "pico/stdio.h"
#include "pico/time.h"

// #include <stdio.h>

#define PAYLOAD_SIZE 128
// Constants - UPDATE THIS SECTION
#define MQTTServerPort 1883 ///< MQTT server Port
#define MQTTServerUser "testuser"
#define MQTTServerPassword "testpassword"
#define KeepAliveSeconds 30

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
  if (MQTT_BROKER_HOSTNAME != NULL) {
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

int main() {
  int publish_interval_sec = 60;
  int delay_sec = 10;
  mqtt_client_t *mc;
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

  while (1) {
    // This is also for wifi disconnect retry etc
    sleep_with_output(delay_sec, "this event loop iteration");

    int rc;
    if ((rc = initialize_wifi()) != 0) {
      printf_ts("initialize_wifi() failed, rc: %d\n", rc);
      if (rc == -2)
        goto err_wifi_initialized_but_connection_failed;
    }

    if ((rc = init_server_ip()) != 0) {
      printf_ts("init_server_ip() failed, rc: %d", rc);
      goto err_init_server_ip_failed;
    }

    memset(&ci, 0, sizeof(ci));
    ci.client_id = MQTT_CLIENT_ID;
    ci.client_user = MQTTServerUser;
    ci.client_pass = MQTTServerPassword;
    ci.keep_alive = KeepAliveSeconds;
    // ClientInfo.will_topic = LwtTopic;
    // ClientInfo.will_msg = LwtMessage;
    // ci.will_qos = 1;
    // ClientInfo.will_retain = LwtRetain;

    cyw43_arch_lwip_begin();

    mqtt_set_inpub_callback(mc, mqtt_incoming_publish_cb, mqtt_incoming_data_cb,
                            LWIP_CONST_CAST(void *, &mc));
    mqtt_err =
        mqtt_client_connect(mc, &ServerIP, MQTTServerPort, mqtt_connection_cb,
                            LWIP_CONST_CAST(void *, &ci), &ci);
    cyw43_arch_lwip_end();
    if (mqtt_err != ERR_OK && mqtt_err != ERR_ISCONN) {
      printf_ts("mqtt_client_connect() failed: %d\n", mqtt_err);
      goto err_mqtt_client_connect_failed;
    }

    // Waiting for the MQTT connection to establish
    sleep_ms(delay_sec * 1000);
    if (mqtt_client_is_connected(mc) == 0) {
      printf_ts("mqtt NOT connected\n");
      goto err_mqtt_client_connect_failed;
    } else {
      printf_ts("mqtt connected\n");
    }

    printf_ts("Entering publishing loop, interval: %d sec\n",
              publish_interval_sec);
    uint8_t counter = 0;
    while (1) {
      sleep_ms(1000);

      cyw43_arch_poll();
      char payload[PAYLOAD_SIZE];

      snprintf(payload, PAYLOAD_SIZE, "{\"reading\": %u}", counter++);
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
    }
  err_mqtt_publish_failed:
    mqtt_disconnect(mc);
  err_mqtt_client_connect_failed:
  err_init_server_ip_failed:
  err_wifi_initialized_but_connection_failed:
    continue;
  }
  cyw43_arch_deinit();
err_mqtt_client_new_failed:
err_cyw43_arch_init_failed:
  return 0;
}
