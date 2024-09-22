#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "pico/cyw43_arch.h"
#include "pico/stdio.h"

#define PAYLOAD_SIZE 128
// Constants - UPDATE THIS SECTION
#define MQTTServerDNS                                                          \
  NULL ///< MQTT server DNS name, leave NULL to use MQTTServerIP instead
#define MQTTServerPort 1883 ///< MQTT server Port
#define MQTTServerUser "testuser"
#define MQTTServerPassword "testpassword"
#define ClientID "Hempy"
#define PubTopic "test_topic"
#define PublishRetain 0
#define QoS 1
#define KeepAliveSeconds 30

bool dnsLookupInProgress = false;
ip_addr_t ServerIP;

typedef void (*Callback_type)(const u8_t *Data, u16_t Len);

extern size_t strlen(const char *s);
extern char *strcat(char *dst, const char *src);
extern void *memset(void *__s, int __c, size_t __n);
extern int printf(const char *__restrict __format, ...);

void mqttPublish_Callback(void *Arg, err_t Result) {
  if (Result != ERR_OK) {
    printf("Publish error: %d\n", Result);
  } else {
    printf("done\n");
  }
}

void mqttReceivedData(const u8_t *Data, u16_t Len) {
  printf("%s\n", Data); // Print the message received on the subscribed topic
}

void mqttIpFound(const char *Hostname, const ip_addr_t *Ipaddr,
                 void *Arg) // DNS lookup callback
{
  dnsLookupInProgress = false;
  if (Ipaddr) {
    printf("Found address: %s\n", ipaddr_ntoa(Ipaddr));
    ServerIP = *Ipaddr;
  } else {
    printf("DNS lookup failed\n");
  }
}

char *intToChar(int Number) // Converting int to char
{
  static char ReturnChar[8] = ""; // 7 digits + null terminator max
  itoa(Number, ReturnChar, 10);
  return ReturnChar;
}

void mqttIncomingTopic_Callback(void *Arg, const char *Topic, u32_t Tot_len) {
  printf("Incoming topic: %s ,total length: %u\n", Topic,
         (unsigned int)Tot_len);
}

void mqttIncomingData_Callback(void *Arg, const u8_t *Data, u16_t Len,
                               u8_t Flags) {
  printf("Incoming payload with length %d, flags %u\n", Len,
         (unsigned int)Flags);

  if (Flags ==
      1) // Last fragment of payload received (or the whole payload fits receive
         // buffer (MQTT_VAR_HEADER_BUFFER_LEN, MQTT_DATA_FLAG_LAST)
  {
    ((Callback_type)Arg)(Data, Len);
  } else {
    printf(
        "Max payload exceeded"); /// TODO: Implement multiple package handling
  }
}

void mqtt_connection_cb(mqtt_client_t *Client, void *Arg,
                        mqtt_connection_status_t Status) {
  if (Status == MQTT_CONNECT_ACCEPTED) {
    printf("mqtt_connection_cb(): MQTT_CONNECT_ACCEPTED\n");
    mqtt_set_inpub_callback(Client, mqttIncomingTopic_Callback,
                            mqttIncomingData_Callback, Arg);
  } else if (Status == MQTT_CONNECT_DISCONNECTED) {
    printf("mqtt_connection_cb(): MQTT_CONNECT_DISCONNECTED\n");
  } else {
    printf("mqtt_connection_cb() error, status code: %d\n", Status);
  }
}

int initialize_wifi() {
  printf("Initializing WiFi\n");

  int rc;

  cyw43_arch_enable_sta_mode();
  printf("Connecting to Hotspot: [%s]...\n", WIFI_SSID);
  if ((rc = cyw43_arch_wifi_connect_timeout_ms(
           WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) != 0) {
    printf("cyw43_arch_wifi_connect_timeout_ms() failed, rc: %d\n", rc);
    return -1;
  }
  printf("wifi connected\n");
  return 0;
}

int init_server_ip() {
  if (MQTTServerDNS !=
      NULL) // If an MQTT server DNS name is specified -> Look up the IP
  {
    printf("Looking up IP for %s...", 0);
    err_t err = dns_gethostbyname(MQTTServerDNS, &ServerIP, mqttIpFound, NULL);
    if (err == ERR_OK) // DNS name found in cache
    {
      printf("Found cached address\n");
    } else {
      dnsLookupInProgress = true;
    }
    absolute_time_t t0 = get_absolute_time();
    while (dnsLookupInProgress) // Waiting for the DNS lookup to finish
    {
      sleep_ms(100);
      if (absolute_time_diff_us(t0, get_absolute_time()) >
          10000000) // 10sec timeout
      {
        printf("DNS lookup timeout\n");
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

  int delay_sec = 10;
  mqtt_client_t *mc;
  absolute_time_t t0;
  struct mqtt_connect_client_info_t ci;
  err_t mqtt_err;

  stdio_init_all();
  if (cyw43_arch_init() != 0) {
    printf("cyw43_arch_init() failed, wifi not initialized\n");
    goto err_cyw43_arch_init_failed;
  }
  mc = mqtt_client_new();
  if (mc == NULL) {
    printf("mqtt_client_new() failed\n");
    goto err_mqtt_client_new_failed;
  }

  while (1) {
    for (int i = 0; i < delay_sec; ++i) {
      printf("Waiting for %d sec before start...\n", delay_sec - i);
      sleep_ms(1000);
    }

    int rc;
    if ((rc = initialize_wifi()) != 0) {
      printf("initialize_wifi() failed, rc: %d\n", rc);
      if (rc == -2)
        goto err_wifi_initialized_but_connection_failed;
    }

    if ((rc = init_server_ip()) != 0) {
      printf("init_server_ip() failed, rc: %d", rc);
      goto err_init_server_ip_failed;
    }

    memset(&ci, 0, sizeof(ci));
    ci.client_id = ClientID;
    ci.client_user = MQTTServerUser;
    ci.client_pass = MQTTServerPassword;
    ci.keep_alive = KeepAliveSeconds;
    // ClientInfo.will_topic = LwtTopic;
    // ClientInfo.will_msg = LwtMessage;
    ci.will_qos = QoS;
    // ClientInfo.will_retain = LwtRetain;

    cyw43_arch_lwip_begin();
    mqtt_err =
        mqtt_client_connect(mc, &ServerIP, MQTTServerPort, mqtt_connection_cb,
                            (void *)mqttReceivedData, &ci);
    cyw43_arch_lwip_end();
    if (mqtt_err != ERR_OK) {
      printf("mqtt_client_connect() failed: %d\n", mqtt_err);
      goto err_mqtt_client_connect_failed;
    }

    t0 = get_absolute_time();
    // Waiting for the MQTT connection to establish
    sleep_ms(delay_sec * 1000);
    if (mqtt_client_is_connected(mc) == 0) {
      printf("mqtt NOT connected\n");
      goto err_mqtt_client_connect_failed;
    }

    while (1) {
      sleep_ms(100);
      if (absolute_time_diff_us(t0, get_absolute_time()) > 3000000) // 3sec
      {
        char payload[PAYLOAD_SIZE];
        snprintf(payload, PAYLOAD_SIZE,
                 "Time: T+%lldms, humidity: %d, speed: %d",
                 get_absolute_time() / 1000, rand() % 100, rand() % 10);
        printf("payload: %s\n", payload);

        printf("Publishing data to %s ...", PubTopic);
        if ((mqtt_err = mqtt_publish(mc, PubTopic, payload, strlen(payload),
                                     QoS, PublishRetain, mqttPublish_Callback,
                                     (void *)&PubTopic)) != ERR_OK) {
          printf("mqtt_publish() failed: %d\n", mqtt_err);
          goto err_mqtt_publish_failed;
        }

        t0 = get_absolute_time();
      }
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
