#pragma once

#include <iostream>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>
#include "Sensors.h"
#include <vector>

class SensorsWebServer {
  public:
  SensorsWebServer(std::vector<Sensor*>& sensors, int port = 8080): server(port), sensors(sensors) {
    server.on("/", [this]() {
      this->handleRoot();
    });
    server.on(UriBraces("/sensors/{}/toggle"), [this]() {
      this->handleToggleSensor();
    });
    server.on(UriBraces("/sensors/{}/changeTime"), [this]() {
      this->handleChangeTimeCycleOfSensor();
    });
  }
  void renderSensorsPage() {
    String sensorsContent = "<html><body><h1>Room Monitoring</h1>";
    sensorsContent += "<table>";
    sensorsContent += "<tr><th>Name</th><th>Is Enabled</th><th>Toggle</th><th>Change Time</th></tr>";
    for (size_t i = 0; i < sensors.size(); i++) {
      sensorsContent += "<tr>";
      sensorsContent += (String("<td> ") + i + " - " + sensors[i]->getName() +"</td>");
      sensorsContent += (String("<td>") + sensors[i]->getIsEnabled() + "</td>");
      sensorsContent += (String("<td><a href=\"/sensors/") + i + "/toggle\">Toggle</a></td>");
      sensorsContent += (String("<td><form action=\"/sensors/") + i + "/changeTime\"><input name=\"cycleTime\" type=\"number\" min=\"1\"></input><button type=\"submit\">Change</button></form></td>");
      sensorsContent += "</tr>";
    }
    sensorsContent += "</table></body></html>";
    server.send(200, "text/html", sensorsContent);
  }

  void handleRoot() {
    renderSensorsPage();
  }

  void handleToggleSensor() {
    unsigned long sensorIndex = server.pathArg(0).toInt();
    Sensor* sensor = sensors[sensorIndex];
    if (sensor->getIsEnabled()) {
      sensor->disable();
    } else {
      sensor->enable();
    }
    
  }

  void handleChangeTimeCycleOfSensor() {
    unsigned long sensorIndex = server.pathArg(0).toInt();
    Sensor* sensor = sensors[sensorIndex];
    String argument = server.arg("cycleTime");
    unsigned long cycleTime = argument.toInt() * 1000;

    sensor->setCycleTime(cycleTime);
    renderSensorsPage();
  }

  void start() {
    this->server.begin();
  }

  void handleClient() {
    this->server.handleClient();
  }

private: 
  WebServer server;
  std::vector<Sensor*>& sensors;
};
