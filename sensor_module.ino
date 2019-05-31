#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <PubSubClient.h>
#include "secrets.h"

#define DHTTYPE DHT11
const int DHTPin = 15;
const int MOTION_PIN = 23;
const int READING_DELAY = 5000;
const int RECONNECT_DELAY = 5000;
const bool MOTION_ENABLED = false;


DHT dht(DHTPin, DHTTYPE);
WiFiClient wifi_client;
PubSubClient client(wifi_client);

float tempC;
float tempF;
float humidity;

void motionDetected() {
  Serial.println("Motion detected!");
}

void setup() {
  Serial.begin(9600);
  Serial.println("Sensor module project!");
  if (MOTION_ENABLED == true)
    attachInterrupt(digitalPinToInterrupt(MOTION_PIN), motionDetected, CHANGE);
  connectToWifi();
  dht.begin();
  client.setServer(mqtt_server, mqtt_port);
}

void connectToWifi() {
  delay(4000);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  };

  Serial.println("Connected to the WiFi network");
}

char* floatToChar(float val) {
  char newString[8];
  return dtostrf(val, 6, 2, newString);
}

char* stringTocharStar(String str) {
  if (str.length() != 0) {
    char *p = const_cast<char*>(str.c_str());
    return p;
  }
}

void publishReadings(float tempC, float tempF, float humidity) {
  // Add ssid to include location information
  if (client.connected()) {
    client.publish("readings/temperature", stringTocharStar(String(tempC)));
    client.publish("readings/humidity", stringTocharStar(String(humidity)));
    Serial.println("messages published");
  } else {
    Serial.println("Failed to publish message: not connected to server");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT server...");
    if (client.connect("ESP32Client", MQTTUSERNAME, MQTTPASSWORD)) {
      Serial.println("Connection to MQTT server successful");
    } else {
      Serial.print("failed to connect to MQTT server, error: ");
      Serial.print(client.state());
      delay(RECONNECT_DELAY);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  delay(READING_DELAY);

  tempC = dht.readTemperature();
  tempF = dht.readTemperature(true);
  humidity = dht.readHumidity();


  Serial.print("DHT temperature reading(C): ");
  Serial.print(tempC);

  Serial.print(", DHT temperature reading(F): ");
  Serial.print(tempF);

  Serial.print(", DHT humidity reading: ");
  Serial.println(humidity);
  publishReadings(tempC, tempF, humidity);
}
