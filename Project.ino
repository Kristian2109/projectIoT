#include <iostream>
#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <HTTPClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Sensors.h"
#include "Web.h"

#include <NeoPixelBus.h>

#include <vector>

#define SEALEVELPRESSURE_HPA (1013.25);

const RgbColor Red = RgbColor(255,0,0);
const RgbColor Green = RgbColor(0,255,0);
const RgbColor Blue = RgbColor(0,0,255);
const RgbColor NonColor = RgbColor(0,0,0);

const int MAX_SSID_LENGTH = 32;
const int MAX_PASSWORD_LENGTH = 64;

char ssid[MAX_SSID_LENGTH];
char password[MAX_PASSWORD_LENGTH];

const char* graphanaUrl = "https://influx-prod-24-prod-eu-west-2.grafana.net/api/v1/push/influx/write";
const int httpPort = 80; 
const char* API_KEY="";

const uint16_t PixelCount = 16; // make sure to set this to the number of pixels in your strip
const uint16_t PixelPin = 17; 

WiFiMulti WiFiMulti;
WiFiClientSecure *client = new WiFiClientSecure;

Adafruit_BME280 bme;
NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> strip(PixelCount, PixelPin); 

std::vector<Sensor*> sensors;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
String header;
unsigned long timeoutTime = 5000;

SensorsWebServer server(sensors, 8080);

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  strip.Begin();
  strip.Show();
  
  initWiFi();

  initBME();

  sensors.push_back(new TemperatureSensor(bme, 10 * 1000, Red));
  sensors.push_back(new HumiditySensor(bme, 20 * 1000, Green));
  sensors.push_back(new PressureSensor(bme, 30 * 1000, Blue));
  
  server.start();
}

void loop() {
  server.handleClient();
  for (Sensor* sensor: sensors) {
    if (sensor->shouldRun()) {
      runSensor(sensor);
    }
  }

  delay(1000);
}

void runSensor(Sensor* sensor) {
      String payload = sensor->createPayload();
      sendDataToGraphanaWithHttp(payload);
      Serial.println(payload);
      Serial.println();
      sensor->setPreviousTime(millis());
      blinkStrip(sensor);
}

void blinkStrip(Sensor* sensor) {
    for (size_t i = 0; i < PixelCount; i++) {
      strip.SetPixelColor(i, sensor->getColor());
    }
    strip.Show();
    delay(500);
    for (size_t i = 0; i < PixelCount; i++) {
      strip.SetPixelColor(i, NonColor);
    }

    strip.Show();
}

void sendDataToGraphanaWithHttp(const String& payload) {
  if (client) {
    client->setInsecure();
    {
      HTTPClient httpClient;
      httpClient.setAuthorizationType("Bearer");
      httpClient.setAuthorization(API_KEY);
      httpClient.addHeader("Content-Type", "text/plain");
      if (httpClient.begin(*client, graphanaUrl)) {
        Serial.print("Beginng a Post request...\n");
        int httpCode = httpClient.POST(payload);

        if (httpCode > 0) {
          Serial.print("Request handled successfully!");
        } else {
          Serial.printf("[HTTPS] GET... failed, error: %s\n", httpClient.errorToString(httpCode).c_str());
        }
      } else {
          Serial.printf("[HTTPS] Unable to connect\n");
      }
    }
  } else {
    Serial.println("Unable to create client");
  }
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  readWiFiCredentials();
  WiFiMulti.addAP(ssid, password);

  while ((WiFiMulti.run() != WL_CONNECTED)) {
    Serial.print(".");
  }
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(" Wi-fi connected");
  delay(5000);
}

void readWiFiCredentials() {
  char empty[MAX_SSID_LENGTH] = "";
  Serial.readBytesUntil('\n', empty, MAX_SSID_LENGTH);

  Serial.println("Enter WiFi SSID:");
  delay(100);
  while (!Serial.available()) {
    delay(10);
  }
  Serial.readBytesUntil('\n', ssid, MAX_SSID_LENGTH);

  // Read password from Serial
  Serial.println("Enter WiFi password:");
  while (!Serial.available()) {
    delay(10);
  }
  Serial.readBytesUntil('\n', password, MAX_PASSWORD_LENGTH);
  Serial.println("SSID: ");
  Serial.println(ssid);
  Serial.println("Password: ");
  Serial.println(password);

  Serial.println("Connecting to WiFi...");
}

void initBME() {
  Serial.println("BME starting...");
  unsigned bmeStatus = 0;

  bmeStatus = bme.begin(0x76);
  if (!bmeStatus) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
  }
  Serial.println("BME started!");
}
