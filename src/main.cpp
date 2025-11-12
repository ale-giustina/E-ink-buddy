#include <Arduino.h>
#include "secrets.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <tr_api.h>
#include <time.h>
#include <weather.h>
#include <helpers.h>
#include <graphics.h>

// Pin definitions
const int led_pins[] = {22, 21, 32, 33, 26, 27};
const int input_pins[] = {15,2,4};

// Global structures
Weather_5D buf_5d;
Weather_24H buf_24h;
Weather_now buf_now;

// Mutexes for thread safety
SemaphoreHandle_t weather_mutex;
SemaphoreHandle_t route_mutex;

// Route information (10 + 6 that can be shifted)
#define MAX_ROUTES 16
RouteInfo info_SMM[MAX_ROUTES];
RouteInfo info_SMM_filtered[MAX_ROUTES];

// Machine states
enum machine_state{
  DEFAULT_CLOCK, DAY_FORECAST_5, BUS_ARRIVALS_5, BUS_ARRIVALS, GRAPH_5_DAYS, GRAPH_24_H
};
machine_state m_state;


// Selector for shifting routes
int shift_selector = 0;

// Graphics update flags
volatile bool graphics_update_in_progress = false;
volatile bool graphics_update_requested = false;

// Five minute display mode flag
volatile bool five_min_display_mode = false;

// Task declarations
void the_timekeeper_tsk(void * parameter);
void api_update_tsk(void * parameter);
void renderer_tsk(void * parameter);
void check_wifi_connection(void * parameter);
void modify_leds(void * parameter);

void setup() {

  // Initial state
  m_state = DEFAULT_CLOCK;

  start_graphics();
  
  // Display startup image
  do{
    display.fillScreen(GxEPD_BLACK);
    display.drawBitmap(D_WIDTH/2 - 75, D_HEIGHT/2 - 75, epd_bitmap_allArray[15], 150, 150, GxEPD_WHITE);
  }while(display.nextPage());

  // Initialize serial communication
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Disable debug mode
  enable_debug = false;

  // Create route map
  while(!create_route_map()){
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  };

  // Initialize LED pins
  for(int i : led_pins){
    pinMode(i, OUTPUT);
  }
  for(int i : input_pins){
    pinMode(i, INPUT);
  }

  // Configure timezone and NTP servers
  configTzTime("CET-1CEST-2,M3.5.0/2,M10.5.0/3", "pool.ntp.org");

  // Create mutexes
  weather_mutex = xSemaphoreCreateMutex();
  route_mutex = xSemaphoreCreateMutex();

  // Start first API update task
  xTaskCreate(api_update_tsk, "API Update Task", 12288, NULL, 1, NULL);

  vTaskDelay(8000 / portTICK_PERIOD_MS);

  // Start renderer task
  xTaskCreate(renderer_tsk, "Renderer Task", 8192, NULL, 1, NULL);

  vTaskDelay(8000 / portTICK_PERIOD_MS);

  // Start timekeeper task
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
  graphics_update_in_progress = true;
  graphics_update_requested = false;
  Serial.print("Rendering... ");
  Serial.printf("Free heap:  %u\n", esp_get_free_heap_size());
  display.firstPage();
  
  // Call functions based on current state
  do {
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    
    if(xSemaphoreTake(weather_mutex, portMAX_DELAY)==pdTRUE){
      draw_time_strip(0, 0, timeinfo, buf_now, &buf_24h, five_min_display_mode);
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
        draw_5_day_forecast(buf_5d, timeinfo, shift_selector);
        xSemaphoreGive(weather_mutex);
      }
    }
    else if(m_state == BUS_ARRIVALS_5){
      if(xSemaphoreTake(route_mutex, portMAX_DELAY)==pdTRUE){
        draw_bus_arrivals(info_SMM_filtered, MAX_ROUTES, shift_selector);
        xSemaphoreGive(route_mutex);
      }
    }
    else if(m_state == BUS_ARRIVALS){
      if(xSemaphoreTake(route_mutex, portMAX_DELAY)==pdTRUE){
        draw_bus_arrivals(info_SMM, MAX_ROUTES, shift_selector);
        xSemaphoreGive(route_mutex);
      }
    }
    else if(m_state == GRAPH_5_DAYS){
      if(xSemaphoreTake(weather_mutex, portMAX_DELAY)==pdTRUE){
        draw_5_day_graphs(buf_5d, timeinfo, shift_selector);
        xSemaphoreGive(weather_mutex);
      }
    }
    else if(m_state == DEFAULT_CLOCK){
      draw_big_time(timeinfo, five_min_display_mode);
    }

  } while (display.nextPage());
  graphics_update_in_progress = false;
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

bool isChoosingStates = false;
bool pressedStates = false;

int wait_second = 0;
int old_secs = -1;
bool update_leds = false;

void modify_leds(void * parameter){

  machine_state working_states[] = {DEFAULT_CLOCK, BUS_ARRIVALS_5, BUS_ARRIVALS, DAY_FORECAST_5, GRAPH_5_DAYS, GRAPH_24_H};

  int secs = *((int*)parameter);

  // Toggle five minute display mode
  if(digitalRead(input_pins[2])){
    five_min_display_mode = !five_min_display_mode;
    graphics_update_requested = true;
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  // Handle shift selection input
  if(digitalRead(input_pins[1])){
    isChoosingShift = true;
    isChoosingStates = false;
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

  // Handle state selection input
  if(digitalRead(input_pins[0])){
    isChoosingStates = true;
    isChoosingShift = false;
    if(!pressedStates){
      int current_state_index = 0;
      for(int i = 0; i<sizeof(working_states)/sizeof(machine_state); i++){
        if(m_state == working_states[i]){
          current_state_index = i;
        }
      }
      shift_selector = 0;
      current_state_index+=1;
      current_state_index=current_state_index%(sizeof(working_states)/sizeof(machine_state));
      m_state = working_states[current_state_index];
      Serial.print("Changed state to ");
      Serial.println(m_state);
      update_leds=true;

    }
    wait_second = (secs+3)%60;
    pressedStates=true;
  }
  else{
    pressedStates=false;
  }

  // Exit selection visualization after timeout
  if(secs == wait_second){
    isChoosingShift=false;
    isChoosingStates=false;
    wait_second = -1;
    graphics_update_requested = true;
  }

  // Update LEDs based on current selection
  if(isChoosingShift){
    if(update_leds){
      for(int i = 0; i<=5; i++){
        if(i<shift_selector){
          analogWrite(led_pins[i], 200);
        }
        else{
          analogWrite(led_pins[i], 0);
        }
      }
      update_leds=false;
    }
  }
  else if(isChoosingStates){
    if(update_leds){
      int state_index = 0;
      for(int i = 0; i<sizeof(working_states)/sizeof(machine_state); i++){
        if(m_state == working_states[i]){
          state_index = i;
        }
      }
      for(int i = 0; i<=5; i++){
        if(i==state_index){
          analogWrite(led_pins[i], 200);
        }
        else{
          analogWrite(led_pins[i], 0);
        }
      }
      update_leds=false;
    }
  }
  else{
    
    if(secs!=old_secs){
      old_secs=secs;
      // Update LEDs based on seconds
      if(!five_min_display_mode){
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
      else{
        struct tm timeinfo;
        getLocalTime(&timeinfo);
        //analogWrite(led_pins[5], secs); //If you want to show seconds with the sixth LED

        // Update LEDs based on minutes
        analogWrite(led_pins[5], 0); 
        for(int i = 5; i>=0; i--){
          
          if(i==timeinfo.tm_min%5){
            if(i-1>=0) analogWrite(led_pins[i-1], 100);
          }
          else if(i<timeinfo.tm_min%5){
            if(i-1>=0) analogWrite(led_pins[i-1], 10);
          }
          else{
            if(i-1>=0) analogWrite(led_pins[i-1], 0);
          }

        }
      }
    }
  }

}

void the_timekeeper_tsk(void * parameter){

  struct tm prev_sec;
  struct tm now;
  getLocalTime(&prev_sec);

  for(;;){
    getLocalTime(&now);

    // Check for time anomalies and restart if detected
    if(now.tm_sec > prev_sec.tm_sec+4 && now.tm_min == prev_sec.tm_min && now.tm_hour == prev_sec.tm_hour){
      WiFi.disconnect();
      ESP.restart();
    }

    // Handle tasks based on the current second
    if(now.tm_sec != prev_sec.tm_sec){
      prev_sec = now;
      // At 30 seconds, update API data
      if(now.tm_sec == 30){
        xTaskCreate(api_update_tsk, "API Update Task", 12288, NULL, 1, NULL);
      }
      // Every minute, request graphics update if in default clock mode
      else if((now.tm_sec == 0 || graphics_update_requested) && !graphics_update_in_progress && !isChoosingShift && !isChoosingStates && !five_min_display_mode){
        xTaskCreate(renderer_tsk, "Renderer Task", 8192, NULL, 1, NULL);
      }
      // At 15 seconds, check WiFi connection
      else if(now.tm_sec == 15){
        xTaskCreate(check_wifi_connection, "WiFi Check Task", 4096, NULL, 1, NULL);
      }
      // Every five minutes, update graphics in five minute display mode
      else if(((now.tm_min % 5 == 0 && now.tm_sec == 0) ||graphics_update_requested) && !graphics_update_in_progress && !isChoosingShift && !isChoosingStates && five_min_display_mode){
        xTaskCreate(renderer_tsk, "Renderer Task", 8192, NULL, 1, NULL);
      }
      
      Serial.println(String("Timekeeper: ")+String(now.tm_hour)+":"+String(now.tm_min)+":"+String(now.tm_sec));

    }
    // Update LEDs
    modify_leds((void*)&now.tm_sec);

  }

}

// Not needed
void loop() {

  vTaskDelete(NULL);

}
