#include <Arduino.h>
#include "secrets.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <map>
#include <tr_api.h>



void setup() {

  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  create_route_map();

  RouteInfo info[5];
  enable_debug = true;
  get_stop_info(407, info, 5);

  for(int i = 0; i < 5; i++) {
    Serial.print("Route: ");
    Serial.print(info[i].shortName);
    Serial.print(", ");
    Serial.print(info[i].longName);
    Serial.print(", Delay: ");
    Serial.print(info[i].delay);
    Serial.print(" min, ETA: ");
    Serial.println(info[i].eta);
  }

  memset(info, 0, sizeof(info));

  get_stop_info(407, info, 5, 5);

  for(int i = 0; i < 5; i++) {
    Serial.print("Route: ");
    Serial.print(info[i].shortName);
    Serial.print(", ");
    Serial.print(info[i].longName);
    Serial.print(", Delay: ");
    Serial.print(info[i].delay);
    Serial.print(" min, ETA: ");
    Serial.println(info[i].eta);
  }

}


void loop() {
  vTaskDelete(NULL);
}
