#include <WiFiClientSecure.h>
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
const int READING_DELAY = 600000;
const int WIFI_RECONNECT_DELAY = 4000;
const int MQTT_RECONNECT_DELAY = 5000;
const bool MOTION_ENABLED = true;
const String SENSOR_ID = "31343DFB-4CB8-4985-8322-ACEA8F26CDC3";
const String SENSOR_NAME = "Jack\'s sensor";

const char* TEMPERATURE_TOPIC = "readings/temperature";
const char* HUMIDITY_TOPIC = "readings/humidity";
const char* MOTION_DETECTED_TOPIC = "alerts/motion_detected";

// NTP settings for time retrieval
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

DHT dht(DHT_PIN, DHTTYPE);
WiFiClientSecure wifi_client;
PubSubClient client(wifi_client);

float tempC;
float tempF;
float humidity;
String readingTime;
int lastReading = READING_DELAY;
bool motionDetectedFlag = false;

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
  motionDetectedFlag = true;
}

void setup() {
  Serial.begin(9600);
  Serial.println("Sensor module project!");
  connectToWifi();
  wifi_client.setCACert(ROOT_CA);
  wifi_client.setCertificate(CLIENT_CERT);
  wifi_client.setPrivateKey(CLIENT_KEY);
  dht.begin();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if (MOTION_ENABLED == true) {
    pinMode(MOTION_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(MOTION_PIN), motionDetected, RISING);
  }
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

DynamicJsonDocument createBaseMessage(String dateTime) {
  DynamicJsonDocument doc(1024);
  doc["sensor_id"] = SENSOR_ID;
  doc["sensor_name"] = SENSOR_NAME;
  doc["event_datetime"] = dateTime;
  return doc;
}

String createHumidityMessage(String dateTime, float humidity) {
  String message;
  DynamicJsonDocument doc = createBaseMessage(dateTime);

  doc["event_type"] = "humidity_reading";
  doc["humidity_percentage"] = humidity;
  serializeJson(doc, message);
  return message;
}

String createTemperatureMessage(String dateTime, float tempC, float tempF) {
  String message;
  DynamicJsonDocument doc = createBaseMessage(dateTime);
  doc["event_type"] = "temperature_reading";
  doc["temperature_celsius"] = tempC;
  doc["temperature_farenheit"] = tempF;
  serializeJson(doc, message);
  return message;
}

String createMotionDetectedMessage() {
  String message;
  String datetime = getTime();
  DynamicJsonDocument doc = createBaseMessage(datetime);
  doc["event_type"] = "motion_detected";
  serializeJson(doc, message);
  return message;
}

void publishMotionDetectedMessage() {
  String message = createMotionDetectedMessage();
  if (client.connected()) {
    publishMessage(MOTION_DETECTED_TOPIC, stringTocharStar(message));
  } else {
    Serial.println("Failed to publish messages: not connected to MQTT server");
  }
}

void publishMessage(char const* topic, char* message) {
  if (client.publish(topic, stringTocharStar(message))) {
    Serial.println("Message published to " + String(topic) + ": " + message);
  } else {
    Serial.println("Problem publishing message to " + String(topic) + ": " + message);
  }
}

void printReadings(String dateTime, float tempC, float tempF, float humidity) {
  Serial.println(dateTime);
  Serial.print("DHT temperature reading(C): ");
  Serial.print(tempC);

  Serial.print(", DHT temperature reading(F): ");
  Serial.print(tempF);

  Serial.print(", DHT humidity reading: ");
  Serial.println(humidity);
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

  if ((millis() - lastReading) > READING_DELAY) {
    String time = getTime();
    tempC = dht.readTemperature();
    tempF = dht.readTemperature(true);
    humidity = dht.readHumidity();

    printReadings(time, tempC, tempF, humidity);
    publishReadings(time, tempC, tempF, humidity);
    lastReading = millis();
  }

  if (motionDetectedFlag == true) {
    motionDetectedFlag = false;
    publishMotionDetectedMessage();
  }
}
