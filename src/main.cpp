#include <time.h>
#include <GxEPD2_3C.h>
#include "secrets.h"
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <WiFi.h>

GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT/4> display(GxEPD2_750c(
    /*CS=*/ 5, /*DC=*/ 14, /*RST=*/ 12, /*BUSY=*/ 25));

#define D_WIDTH GxEPD2_750c::WIDTH
#define D_HEIGHT GxEPD2_750c::HEIGHT

void draw_graph(float data[], int len, int x, int y, int w, int h, float min_val, float max_val, uint16_t color, int thickness=3, int x_ticks=5, int y_ticks=5, int x_tick_label_min=0, int x_tick_label_max=10) {
    if (len < 2) return; // Need at least two points to draw a graph

    // Draw axes
    display.drawRect(x, y, w, h, GxEPD_BLACK);
    // Draw ticks and labels on x-axis
    display.setFont(&FreeSans9pt7b);
    for (int i = 0; i <= x_ticks; i++) {
        int xt = x + i * (w / x_ticks);
        display.drawLine(xt, y + h, xt, y + h + 5, GxEPD_BLACK);
        float label = x_tick_label_min + i * (x_tick_label_max - x_tick_label_min) / x_ticks;
        display.setCursor(xt - 10, y + h + 20);
        display.print((int)label, 1);
    }
    // Draw ticks and labels on y-axis
    for (int i = 0; i <= y_ticks; i++) {
        int yt = y + h - i * (h / y_ticks);
        display.drawLine(x - 5, yt, x, yt, GxEPD_BLACK);
        float label = min_val + i * (max_val - min_val) / y_ticks;
        display.setCursor(x - 40, yt + 5);
        display.print(label, 1);
    }

    float x_scale = (float)w / (len - 1);
    float y_scale = (float)h / (max_val - min_val);

    for (int i = 0; i < len - 1; i++) {
        int x0 = x + i * x_scale;
        int y0 = y + h - (data[i] - min_val) * y_scale;
        int x1 = x + (i + 1) * x_scale;
        int y1 = y + h - (data[i + 1] - min_val) * y_scale;


        display.fillCircle(x0, y0, 2, color);
        for (int t = 0; t < thickness; t++) {
            display.drawLine(x0, y0 + t, x1, y1 + t, color);
        }
    }

}
struct Weather_now {

    struct tm last_update = {-1};
    short code;
    float temp;
    float windspeed;
    short precipitation_probability;
    short humidity;
    
};
void draw_time_strip(int x, int y, int w, int h, struct tm &timeinfo, Weather_now &current_weather) {
    // Draw time date and current weather info strip
    char buffer[30];
    display.setTextColor(GxEPD_DARKGREY);
    strftime(buffer, sizeof(buffer), "%H:%M", &timeinfo);
    display.setFont(&FreeMonoBold18pt7b);
    display.setTextSize(2);
    display.setCursor(x + 10, y + 50);
    display.print(buffer);
    display.setTextSize(1);
    strftime(buffer, sizeof(buffer), "%a %d %b %Y", &timeinfo);
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(x + 10, y + 35 + 50);
    display.print(buffer);

    

    // Display current weather info
    display.setCursor(x + 10, y + 35 + 40 + 30);
    display.print(current_weather.temp, 1);
    display.print("C ");
    display.print(current_weather.windspeed, 1);
    display.print("km/h ");
    display.print(current_weather.precipitation_probability);
    display.print("% RH:");
    display.print(current_weather.humidity);
    display.print("%");

}

void setup(){
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    configTzTime("CET-1CEST-2,M3.5.0/2,M10.5.0/3", "pool.ntp.org");
    delay(2000);
    Serial.println("GxEPD2 3-Color Display Test");
    Serial.println(D_HEIGHT);
    Serial.println(D_WIDTH);
    display.init();
    display.setRotation(0); // rotate if needed to fit layout

    float data[10] = {10.5, 12.0, 11.5, 13.0, 14.5, 13.5, 15.0, 16.0, 15.5, 17.0};

    display.setFullWindow();
    display.firstPage();

    struct tm now;
    getLocalTime(&now);
    Weather_now wea;
    wea.temp = 20;
    wea.humidity = 80;
    wea.precipitation_probability=50;
    wea.windspeed=3;

    do {
        display.fillScreen(GxEPD_WHITE);
        draw_time_strip(0,0,D_WIDTH,110,now,wea);
        display.drawLine(0, 120, D_WIDTH, 120, GxEPD_RED);
        draw_graph(data, 10, 60, 130, D_WIDTH - 90, D_HEIGHT - 180, 10.0, 18.0, GxEPD_RED, 3, 10, 4, 0, 9);
    } while (display.nextPage());







}

void loop(){
    vTaskDelete(NULL);
}