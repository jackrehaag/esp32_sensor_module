#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <DHT_U.h>
#include <PubSubClient.h>
#include "time.h"
#include "secrets.h"

#define DHTTYPE DHT11
const int DHT_PIN = 15;
const int MOTION_PIN = 23;
const int READING_DELAY = 5000;
const int WIFI_RECONNECT_DELAY = 4000;
const int MQTT_RECONNECT_DELAY = 5000;
const bool MOTION_ENABLED = false;
const String SENSOR_ID = "Jack\'s sensor";

const char* TEMPERATURE_TOPIC = "readings/temperature";
const char* HUMIDITY_TOPIC = "readings/humidity";

// NTP settings for time retrieval
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

DHT dht(DHT_PIN, DHTTYPE);
WiFiClient wifi_client;
PubSubClient client(wifi_client);

float tempC;
float tempF;
float humidity;
String readingTime;

String getTime() {
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  char timeStringBuff[100];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%FT%H:%M:%S", &timeinfo);
  std::string str(timeStringBuff);
  return str.c_str();
}

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
  client.setServer(MQTT_SERVER, MQTT_PORT);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println(getTime());
}

void connectToWifi() {
  delay(WIFI_RECONNECT_DELAY);
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

String createHumidityMessage(String dateTime, float humidity) {
  DynamicJsonDocument doc(1024);
  String message;

  doc["sensor_id"] = SENSOR_ID;
  doc["event_datetime"] = dateTime;
  doc["event_type"] = "humidity_reading";
  doc["humidity_percentage"] = humidity;
  serializeJson(doc, message);
  return message;
}

String createTemperatureMessage(String dateTime, float tempC, float tempF) {
  DynamicJsonDocument doc(1024);
  String message;

  doc["sensor_id"] = SENSOR_ID;
  doc["event_datetime"] = dateTime;
  doc["event_type"] = "temperature_reading";
  doc["temperature_celsius"] = tempC;
  doc["temperature_farenheit"] = tempF;
  serializeJson(doc, message);
  return message;
}

void publishMessage(char const* topic, char* message) {
  if (client.publish(topic, stringTocharStar(message))) {
    Serial.println("Message published to " + String(topic) + ": " + message);
  } else {
    Serial.println("Problem publishing message to " + String(topic) + ": " + message);
  }
}

void publishReadings(String dateTime, float tempC, float tempF, float humidity) {
  String temperature_message = createTemperatureMessage(dateTime, tempC, tempF);
  String humidity_message = createHumidityMessage(dateTime, humidity);

  if (client.connected()) {
    publishMessage(TEMPERATURE_TOPIC, stringTocharStar(temperature_message));
    publishMessage(HUMIDITY_TOPIC, stringTocharStar(humidity_message));
  } else {
    Serial.println("Failed to publish messages: not connected to MQTT server");
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
      delay(MQTT_RECONNECT_DELAY);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  String time = getTime();
  tempC = dht.readTemperature();
  tempF = dht.readTemperature(true);
  humidity = dht.readHumidity();

  Serial.println(time);

  Serial.print("DHT temperature reading(C): ");
  Serial.print(tempC);

  Serial.print(", DHT temperature reading(F): ");
  Serial.print(tempF);

  Serial.print(", DHT humidity reading: ");
  Serial.println(humidity);
  publishReadings(time, tempC, tempF, humidity);
  delay(READING_DELAY);
}
