#include <Arduino.h>
#include "secrets.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <tr_api.h>
#include <time.h>
#include <weather.h>
#include <helpers.h>

SemaphoreHandle_t weather_mutex;
Weather_5D buf_5d;
Weather_24H buf_24h;
Weather_now buf_now;

SemaphoreHandle_t route_mutex;
RouteInfo info_SMM[5];
RouteInfo info_SMM_filtered[5];

// Task declarations
void the_timekeeper_tsk(void * parameter);
void api_update_tsk(void * parameter);
void renderer_tsk(void * parameter);
void check_wifi_connection(void * parameter);

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

  weather_mutex = xSemaphoreCreateMutex();
  route_mutex = xSemaphoreCreateMutex();

  xTaskCreate(the_timekeeper_tsk, "Timekeeper Task", 8192, NULL, 1, NULL);
  
}

void api_update_tsk(void * parameter){
  
  debug_println("API Update Task started");
  
  if(xSemaphoreTake(weather_mutex, portMAX_DELAY)==pdTRUE){
    get_weather_5d(buf_5d);
    get_weather_24h(buf_24h);
    get_current_weather(buf_now);
    xSemaphoreGive(weather_mutex);
  }

  debug_println("Done weather, fetching route info...");

  if(xSemaphoreTake(route_mutex, portMAX_DELAY)==pdTRUE){
    get_stop_info(407, info_SMM, 5);
    get_stop_info_filtered(407, info_SMM_filtered, 5, 400, false);
    xSemaphoreGive(route_mutex);
  }

  debug_println("==========================");
  debug_print_routes(info_SMM, 5);
  debug_println("==========================");
  debug_print_routes(info_SMM_filtered, 5);
  debug_println("==========================");
  debug_print_weather_5d(buf_5d);
  debug_println("==========================");
  debug_print_weather_24h(buf_24h);
  debug_println("==========================");
  debug_print_weather_now(buf_now);
  debug_println("==========================");
 
  vTaskDelete(NULL);

}

void renderer_tsk(void * parameter){

  Serial.print("Rendering... ");
  Serial.printf("Free heap:  %u\n", esp_get_free_heap_size());
  //rendering code here
  vTaskDelete(NULL);

}

void the_timekeeper_tsk(void * parameter){

  struct tm prev_sec;
  struct tm now;
  getLocalTime(&prev_sec);

  for(;;){
    getLocalTime(&now);
    if(now.tm_sec != prev_sec.tm_sec){
      prev_sec = now;
      if(now.tm_sec == 30){
        xTaskCreate(api_update_tsk, "API Update Task", 12288, NULL, 1, NULL);
      }
      else if(now.tm_sec == 0){
        xTaskCreate(renderer_tsk, "Renderer Task", 8192, NULL, 1, NULL);
      }
      else if(now.tm_sec == 15){
        xTaskCreate(check_wifi_connection, "WiFi Check Task", 4096, NULL, 1, NULL);
      }
      Serial.println(String("Timekeeper: ")+String(now.tm_hour)+":"+String(now.tm_min)+":"+String(now.tm_sec));

    }

  }

}
void check_wifi_connection(void * parameter){
  if(!is_connected()){
    Serial.println("WiFi disconnected, attempting reconnection...");
    WiFi.disconnect();
    WiFi.reconnect();
    unsigned long startAttemptTime = millis();
    // Wait for connection for 10 seconds
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      Serial.print(".");
    }
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("Reconnected to WiFi");
    }
    else{
      Serial.println("Failed to reconnect to WiFi");
    }
  }
  else{
    debug_println("Wifi is fine");
  }
  vTaskDelete(NULL);
}
void loop() {

  vTaskDelete(NULL);

}
