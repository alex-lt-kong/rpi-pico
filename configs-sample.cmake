add_definitions(-DWIFI_SSID="WIFI Name")
add_definitions(-DWIFI_PASSWORD="WifiPa$$w0rd")
# define either IP or HOSTNAME
add_definitions(-DMQTT_BROKER_IP="")
add_definitions(-DMQTT_BROKER_HOSTNAME="mqtt.lan")
add_definitions(-DMQTT_CLIENT_ID="location1-mqtt-device")
add_definitions(-DMQTT_TOPIC="/readings/temperature/location1")