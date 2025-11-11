#include <Arduino.h>
#include "secrets.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <tr_api.h>
#include <time.h>
#include <weather.h>
#include <helpers.h>
#include <graphics.h>

const int led_pins[] = {22, 21, 32, 33, 26, 27};

const int input_pins[] = {15,2,4};

SemaphoreHandle_t weather_mutex;
Weather_5D buf_5d;
Weather_24H buf_24h;
Weather_now buf_now;

SemaphoreHandle_t route_mutex;

#define MAX_ROUTES 12
RouteInfo info_SMM[MAX_ROUTES];
RouteInfo info_SMM_filtered[MAX_ROUTES];

enum machine_state{
  DAY_FORECAST_5, BUS_ARRIVALS_5, BUS_ARRIVALS, GRAPH_5_DAYS, GRAPH_24_H
};
machine_state m_state;

int shift_selector = 0;

// Task declarations
void the_timekeeper_tsk(void * parameter);
void api_update_tsk(void * parameter);
void renderer_tsk(void * parameter);
void check_wifi_connection(void * parameter);
void modify_leds(void * parameter);

void setup() {

  m_state = GRAPH_5_DAYS;

  start_graphics();
  display.setTextSize(3);
  do{
    display.fillScreen(GxEPD_BLACK);
    display.drawBitmap(D_WIDTH/2 - 75, D_HEIGHT/2 - 75, epd_bitmap_allArray[15], 150, 150, GxEPD_WHITE);
  }while(display.nextPage());
  display.setTextSize(1);

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

  for(int i : led_pins){
    pinMode(i, OUTPUT);
  }
  for(int i : input_pins){
    pinMode(i, INPUT);
  }

  configTzTime("CET-1CEST-2,M3.5.0/2,M10.5.0/3", "pool.ntp.org");

  weather_mutex = xSemaphoreCreateMutex();
  route_mutex = xSemaphoreCreateMutex();

  xTaskCreate(api_update_tsk, "API Update Task", 12288, NULL, 1, NULL);

  vTaskDelay(8000 / portTICK_PERIOD_MS);

  xTaskCreate(renderer_tsk, "Renderer Task", 8192, NULL, 1, NULL);

  vTaskDelay(8000 / portTICK_PERIOD_MS);
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
  display.firstPage();
  do {
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    
    if(xSemaphoreTake(weather_mutex, portMAX_DELAY)==pdTRUE){
      draw_time_strip(0, 0, timeinfo, buf_now, &buf_24h);
      xSemaphoreGive(weather_mutex);
    }
    if(m_state == GRAPH_24_H){
      if(xSemaphoreTake(weather_mutex, portMAX_DELAY)==pdTRUE){
        draw_24_h_graphs(buf_24h, timeinfo, shift_selector);
        xSemaphoreGive(weather_mutex);
      }
    }
    else if(m_state == DAY_FORECAST_5){
      if(xSemaphoreTake(weather_mutex, portMAX_DELAY)==pdTRUE){
        draw_5_day_forecast(buf_5d, timeinfo);
        xSemaphoreGive(weather_mutex);
      }
    }
    else if(m_state == BUS_ARRIVALS_5){
      if(xSemaphoreTake(route_mutex, portMAX_DELAY)==pdTRUE){
        draw_bus_arrivals(info_SMM_filtered, MAX_ROUTES);
        xSemaphoreGive(route_mutex);
      }
    }
    else if(m_state == BUS_ARRIVALS){
      if(xSemaphoreTake(route_mutex, portMAX_DELAY)==pdTRUE){
        draw_bus_arrivals(info_SMM, MAX_ROUTES);
        xSemaphoreGive(route_mutex);
      }
    }
    else if(m_state == GRAPH_5_DAYS){
      if(xSemaphoreTake(weather_mutex, portMAX_DELAY)==pdTRUE){
        draw_5_day_graphs(buf_5d, timeinfo, shift_selector);
        xSemaphoreGive(weather_mutex);
      }
    }

  } while (display.nextPage());
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

bool isChoosingShift = false;
bool pressedShift = false;
int wait_second = 0;
int old_secs = -1;
bool update_leds = false;
void modify_leds(void * parameter){

  machine_state working_states[] = {GRAPH_24_H, GRAPH_5_DAYS};

  int secs = *((int*)parameter);

  if(digitalRead(input_pins[0])){
    isChoosingShift = true;
    if(!pressedShift){
      Serial.println(shift_selector);
      shift_selector+=1;
      shift_selector=shift_selector%7;
      update_leds=true;
    }
    pressedShift=true;
    wait_second = (secs+3)%60;
  }
  else{
    pressedShift=false;
  }
  if(secs == wait_second){
    isChoosingShift=false;
  }

  if(isChoosingShift){
    if(update_leds){
      for(int i = 0; i<=5; i++){
        if(i<shift_selector){
          analogWrite(led_pins[i], 100);
        }
        else{
          analogWrite(led_pins[i], 0);
        }
      }
    }
  }
  else{
    
    if(secs!=old_secs){
      Serial.print(secs);
      Serial.print(" ");
      Serial.println(old_secs);
      old_secs=secs;
      for(int i = 5; i>=0; i--){
        //Serial.print(secs&0b1);
        if(secs&0b1){
          analogWrite(led_pins[i], 20);
        }
        else{
          analogWrite(led_pins[i], 0);
        }
        secs=secs>>1;
      }
    }
  }
  //Serial.println();

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
      

      Serial.println(String("Timekeeper: ")+String(now.tm_hour)+":"+String(now.tm_min)+":"+String(now.tm_sec));

    }
    modify_leds((void*)&now.tm_sec);

  }

}

void loop() {

  vTaskDelete(NULL);

}
