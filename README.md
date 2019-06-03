# ESP32 multi-sensor IOT module

IOT sensor project to publish temperature and humidity readings to an MQTT topic.

## Getting started

The `MQTT_MAX_PACKET_SIZE` value in the PubSubClient library will need to be updated from 128 to 256 to allow for adequately sized MQTT messages to be sent. Unfortunately the only way to do this is by editing the `PubSubClient.h` file directly.

### Hardware

- ESP32 development board
- DHT11 temperature and humidity sensor
- HC-SR501 PIR motion sensor (optional)
- 10k resistor

### secrets.h

Rename the `secrets.example.h` file to `secrets.h` and update the variables accordingly.
An explanation of these variables can be found below

| Secret          | Usage                                                                      |
| --------------- | -------------------------------------------------------------------------- |
| `WIFI_SSID`     | SSID of your WiFi network                                                  |
| `WIFI_PASSWORD` | Password for the WiFi network                                              |
| `MQTTUSERNAME`  | Username for an user on the MQTT server that you want to publish to        |
| `MQTTPASSWORD`  | Password for the above user on the MQTT server that you want to publish to |

Other non-secret variables to consider are:

| Variable               | Usage                                                                                             |
| ---------------------- | ------------------------------------------------------------------------------------------------- |
| `DHT_PIN`              | GPIO pin connected to the data pin of the DHT11 sensor                                            |
| `MOTION_PIN`           | GPIO pin connected to the data pin of the motion sensor (optional, see `MOTION_ENABLED` variable) |
| `READING_DELAY`        | Delay between readings (milliseconds)                                                             |
| `WIFI_RECONNECT_DELAY` | Delay between reconnects to the WiFi router if a connection cannot be established                 |
| `MQTT_RECONNECT_DELAY` | Delay between reconnects to the MQTT server if a connection cannot be established                 |
| `MOTION_ENABLED`       | Boolean value to enable/disable the motion sensor readings                                        |
| `SENSOR_NAME`          | A name for the sensor to differentiate between readings from multiple sensors                     |
| `TEMPERATURE_TOPIC`    | MQTT topic to publish temperature readings to                                                     |
| `HUMIDITY_TOPIC`       | MQTT topic to publish humidity readings to                                                        |

## Todo

- Publish motion sensor readings to MQTT
- Add circuit diagram
