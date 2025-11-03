#include <Arduino.h>
#include "secrets.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <tr_api.h>
#include <time.h>
#include <weather.h>
#include <helpers.h>

Weather_5D buf;

void setup() {

  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  enable_debug = true;
  create_route_map();

  configTzTime("CET-1CEST-2,M3.5.0/2,M10.5.0/3", "pool.ntp.org");

  
  

  /*
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
  */
}


void loop() {
  

  get_weather_5d(buf);
  print_timestamp();
  for(int i = 0; i<5; i++){
    Serial.print(" - ");
    Serial.print(buf.codes[i]);
  }
  Serial.println();
  for(int i = 0; i<5; i++){
    Serial.print(" - ");
    Serial.print(buf.precipitation[i]);
  }
  Serial.println();
  for(int i = 0; i<5; i++){
    Serial.print(" - ");
    Serial.print(buf.temp_max[i]);
  }
  Serial.println();
  for(int i = 0; i<5; i++){
    Serial.print(" - ");
    Serial.print(buf.temp_min[i]);
  }
  Serial.println();

  delay(10000);

}
