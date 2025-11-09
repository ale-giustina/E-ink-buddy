#include <Arduino.h>
#include "secrets.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <tr_api.h>
#include <time.h>
#include <weather.h>
#include <helpers.h>

const int led_pins[] = {1,2,3,4,5,6}; //example led pins

SemaphoreHandle_t weather_mutex;
Weather_5D buf_5d;
Weather_24H buf_24h;
Weather_now buf_now;

SemaphoreHandle_t route_mutex;

#define MAX_ROUTES 12
RouteInfo info_SMM[MAX_ROUTES];
RouteInfo info_SMM_filtered[MAX_ROUTES];

// Task declarations
void the_timekeeper_tsk(void * parameter);
void api_update_tsk(void * parameter);
void renderer_tsk(void * parameter);
void check_wifi_connection(void * parameter);
void modify_leds(void * parameter);

void setup() {

  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  enable_debug = true;
  while(!create_route_map()){
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  };

  configTzTime("CET-1CEST-2,M3.5.0/2,M10.5.0/3", "pool.ntp.org");

  weather_mutex = xSemaphoreCreateMutex();
  route_mutex = xSemaphoreCreateMutex();

  xTaskCreate(the_timekeeper_tsk, "Timekeeper Task", 8192, NULL, 1, NULL);
  
}

void api_update_tsk(void * parameter){
  
  Serial.println("API Update Task started");

  if(xSemaphoreTake(weather_mutex, portMAX_DELAY)==pdTRUE){
    get_weather_5d(buf_5d);
    Serial.println("Fetched 5-day weather");
    get_weather_24h(buf_24h);
    Serial.println("Fetched 24-hour weather");
    get_current_weather(buf_now);
    Serial.println("Fetched current weather");
    xSemaphoreGive(weather_mutex);
  }
  
  Serial.println("Done weather, fetching route info...");
  
  if(xSemaphoreTake(route_mutex, portMAX_DELAY)==pdTRUE){
    get_stop_info(407, info_SMM, MAX_ROUTES);
    get_stop_info_filtered(407, info_SMM_filtered, MAX_ROUTES, 400, false, true);
    xSemaphoreGive(route_mutex);
  }

  debug_println("==========================", true);
  debug_print_routes(info_SMM, MAX_ROUTES, true);
  debug_println("==========================", true);
  debug_print_routes(info_SMM_filtered, MAX_ROUTES, true);
  debug_println("==========================", true);

  debug_print_weather_5d(buf_5d, true);
  debug_println("==========================", true);
  debug_print_weather_24h(buf_24h, true);
  debug_println("==========================", true);
  debug_print_weather_now(buf_now, true);
  debug_println("==========================", true);
 
  vTaskDelete(NULL);

}

void renderer_tsk(void * parameter){

  Serial.print("Rendering... ");
  Serial.printf("Free heap:  %u\n", esp_get_free_heap_size());
  //rendering code here
  vTaskDelete(NULL);

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
    Serial.println("Wifi is fine");
  }
  vTaskDelete(NULL);
}

void modify_leds(void * parameter){

  int secs = *((int*)parameter);

  for(int i = 5; i>=0; i--){
    //Serial.print(secs&0b1);
    secs=secs>>1;
  }
  //Serial.println();
  vTaskDelete(NULL);

}

void the_timekeeper_tsk(void * parameter){

  struct tm prev_sec;
  struct tm now;
  getLocalTime(&prev_sec);

  for(;;){
    getLocalTime(&now);

    if(now.tm_sec > prev_sec.tm_sec+4 && now.tm_min == prev_sec.tm_min && now.tm_hour == prev_sec.tm_hour){
      WiFi.disconnect();
      ESP.restart();
    }

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
      
      xTaskCreate(modify_leds, "Update led array", 4096, (void*)&now.tm_sec, 1, NULL);

      Serial.println(String("Timekeeper: ")+String(now.tm_hour)+":"+String(now.tm_min)+":"+String(now.tm_sec));

    }

  }

}

void loop() {

  vTaskDelete(NULL);

}
