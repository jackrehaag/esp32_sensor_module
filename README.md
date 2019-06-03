# ESP32 multi-sensor IOT module

IOT sensor project to publish temperature and humidity readings to an MQTT topic.

## Getting started

The `MQTT_MAX_PACKET_SIZE` value in the PubSubClient library will need to be updated from 128 to 256 to allow for adequately sized MQTT messages to be sent. Unfortunately the only way to do this is by editing the `PubSubClient.h` file directly.

### Hardware

- ESP32 development board
- DHT11
- 10k resistor
