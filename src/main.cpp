#include <Arduino.h>
#include "secrets.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <map>
#include <tr_api.h>
#include <time.h>

String time_helper(char c){
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    switch (c) {
      case 'H': return timeinfo.tm_hour < 10 ? "0" + String(timeinfo.tm_hour) : String(timeinfo.tm_hour);
      case 'M': return timeinfo.tm_min < 10 ? "0" + String(timeinfo.tm_min) : String(timeinfo.tm_min);
      case 'S': return timeinfo.tm_sec < 10 ? "0" + String(timeinfo.tm_sec) : String(timeinfo.tm_sec);
      case 'd': return timeinfo.tm_mday < 10 ? "0" + String(timeinfo.tm_mday) : String(timeinfo.tm_mday);
      case 'm': return (timeinfo.tm_mon + 1) < 10 ? "0" + String(timeinfo.tm_mon + 1) : String(timeinfo.tm_mon + 1);
      case 'Y': return String(timeinfo.tm_year + 1900);
      default: return "";
    }
  }
  return "";
}

void print_timestamp(){
  String timestamp = String(time_helper('Y')) + "-" + String(time_helper('m')) + "-" + String(time_helper('d')) + " " +
                     String(time_helper('H')) + ":" + String(time_helper('M')) + ":" + String(time_helper('S')) + String(" - ");
  Serial.print(timestamp);
}

void setup() {

  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  enable_debug = false;
  create_route_map();

  configTzTime("CET-1CEST-2,M3.5.0/2,M10.5.0/3", "pool.ntp.org");

  RouteInfo info[5];
  
  get_stop_info(407, info, 5);

  for(int i = 0; i < 5; i++) {
    print_timestamp();
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

  get_stop_info_filtered(407, info, 5, 400, false);

  for(int i = 0; i < 5; i++) {
    print_timestamp();
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
