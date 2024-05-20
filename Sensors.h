#pragma once

#include <Arduino.h>
#include <iostream>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <NeoPixelBus.h>
#include <cstring>

class Sensor {
protected:
  unsigned long previousActionTime;
  unsigned long cycleTime;
  bool isEnabled;
  RgbColor color;
  char name[20] = "Default";

public:
  Sensor(unsigned long cycleTime, const RgbColor& color) : previousActionTime(0), cycleTime(cycleTime), color(color), isEnabled(true) {}

  bool shouldRun() {
    return this->isEnabled && (millis() - previousActionTime) >= cycleTime;
  }

  virtual String createPayload();

  void setPreviousTime(unsigned long time) {
    this->previousActionTime = time;
  }
  void setName(const char* name) {
    strcpy(this->name, name);
  }
  const char* getName() {
    return this->name;
  }
  void setCycleTime(unsigned long time) {
    this->cycleTime = time;
  }
  void setColor(const RgbColor& color) {
    this->color = color;
  }
  const RgbColor& getColor() {
    return this->color;
  } 
  bool getIsEnabled() {
    return this->isEnabled;
  }
  void enable() {
    this->isEnabled = true;
  }
  void disable() {
    this->isEnabled = false;
  }
};

class BmeSensor : public Sensor {
public:
  BmeSensor(Adafruit_BME280& bme, unsigned long cycleTime, const RgbColor& color) : bme(bme), Sensor(cycleTime, color) {}
protected:
  Adafruit_BME280& bme;
  String getCommonBme() {
    return String(",sensor_model=bme280,sensor_id=") + bme.sensorID() + " metric=";
  }
};

class TemperatureSensor : public BmeSensor {
public:
  TemperatureSensor(Adafruit_BME280& bme, unsigned long cycleTime, const RgbColor& color) : BmeSensor(bme, cycleTime, color) {}
  
  String createPayload() {
    return String("temperature") + this->getCommonBme() + this->bme.readTemperature();
  }
};

class HumiditySensor : public BmeSensor {
public:
  HumiditySensor(Adafruit_BME280& bme, unsigned long cycleTime, const RgbColor& color) : BmeSensor(bme, cycleTime, color) {}
  String createPayload() {
    return String("humidity") + this->getCommonBme() + this->bme.readHumidity();
  }
};

class PressureSensor : public BmeSensor {
public:
  PressureSensor(Adafruit_BME280& bme, unsigned long cycleTime, const RgbColor& color) : BmeSensor(bme, cycleTime, color) {}
  String createPayload() {
    return String("pressure") + this->getCommonBme() + (this->bme.readPressure() / 100.0F);
  }
};
