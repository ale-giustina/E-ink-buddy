#include <graphics.h>

// Initialize the display
GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT/4> display(GxEPD2_750c(
    /*CS=*/ 5, /*DC=*/ 14, /*RST=*/ 12, /*BUSY=*/ 25));


void start_graphics() {
    display.init(); 
    display.setRotation(0); // Landscape
    display.setFullWindow();
}

const int D_WIDTH = GxEPD2_750c::WIDTH; // Screen width
const int D_HEIGHT = GxEPD2_750c::HEIGHT; // Screen height

template<typename T>
void draw_graph(T data[], int len, int x, int y, int w, int h, float min_val, float max_val, uint16_t color, int thickness, int x_ticks, int y_ticks, int x_tick_label_min, int x_tick_label_max, bool noaxis, int x_cyclic) {
    if (len < 2) return; // Need at least two points to draw a graph

    if(!noaxis){
        // Draw axes
        display.drawRect(x, y, w, h, GxEPD_BLACK);

        // Draw axes
        display.drawRect(x, y, w, h, GxEPD_BLACK);
        // Draw ticks and labels on x-axis
        display.setFont(&FreeSans9pt7b);
        for (int i = 0; i <= x_ticks; i++) {
            int xt = x + i * (float)(w / x_ticks);
            display.drawLine(xt, y + h, xt, y + h + 5, GxEPD_BLACK);
            float label = x_tick_label_min + i * (x_tick_label_max - x_tick_label_min) / x_ticks;
            display.setCursor(xt - 10, y + h + 20);
            if (x_cyclic > 0) {
                float cyclic_label = fmod(label, x_cyclic);
                if (cyclic_label < 0) cyclic_label += x_cyclic; // ensure positive
                display.print((int)cyclic_label, 1);
            } else {
                display.print((int)label, 1);
            }
        }
        // Draw ticks and labels on y-axis
        for (int i = 0; i <= y_ticks; i++) {
            int yt = y + h - i * (float)(h / y_ticks);
            display.drawLine(x - 5, yt, x, yt, GxEPD_BLACK);
            float label = min_val + i * (max_val - min_val) / y_ticks;
            display.setCursor(x - 30, yt + 5);
            display.print((int)label, 1);
        }
    }

    float x_scale = (float)w / (len - 1);
    float y_scale = (float)h / (max_val - min_val);

    // Draw data points and lines ensuring floating-point precision
    for (int i = 0; i < len - 1; i++) {
        float fx0 = x + i * x_scale;
        float fy0 = y + h - (float)((float)data[i] - min_val) * y_scale;
        float fx1 = x + (i + 1) * x_scale;
        float fy1 = y + h - (float)((float)data[i + 1] - min_val) * y_scale;
        int x0 = round(fx0);
        int y0 = round(fy0);
        int x1 = round(fx1);
        int y1 = round(fy1);

        display.fillCircle(x0, y0, 2, color);

        // Draw thicker lines by drawing multiple lines offset by 1 pixel
        for (int t = 0; t < thickness; t++) {
            display.drawLine(x0, y0 + t, x1, y1 + t, color);
        }
    }

}

void draw_time_strip(int x, int y, struct tm &timeinfo, Weather_now &current_weather, Weather_24H *forecast_24h, bool five_min_mode) {
    
    // Draw time date and current weather info strip
    
    // Time and date section
    char buffer[30];
    display.setTextColor(GxEPD_BLACK);
    if(five_min_mode){
        timeinfo.tm_min = (timeinfo.tm_min / 5) * 5; // round down to nearest 5
    }
    mktime(&timeinfo); // normalize
    strftime(buffer, sizeof(buffer), "%H:%M", &timeinfo);
    display.setFont(&FreeMonoBold18pt7b);
    display.setTextSize(2);
    display.setCursor(x + 10, y + 50);
    display.print(buffer);
    display.setTextSize(1);
    strftime(buffer, sizeof(buffer), "%a %d %b", &timeinfo);
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(x + 10, y + 35 + 50);
    display.print(buffer);

    // Display current weather info
    display.setCursor(x + 20 + 30*7 , y + 30);
    display.print(current_weather.temp, 1);
    display.print("C");
    display.setCursor(x + 20 + 30*7 , y + 60);
    display.print(current_weather.windspeed, 1);
    display.print("kph");
    display.setCursor(x + 20 + 30*7 , y + 90);
    display.drawBitmap(x + 20 + 30*6 ,  y + 55, epd_bitmap_allArray[16], 100, 100, GxEPD_BLACK);
    display.print("  ");
    display.print(current_weather.humidity);
    display.print("%");
    display.setCursor(x + 20 + 30*7 , y + 120);

    draw_graph(forecast_24h->temperature.data(), 24, 10 + 380, 7, 240, 80, forecast_24h->temp_min, forecast_24h->temp_max, GxEPD_RED, 2, 6, 2, 0, 23,false, 25);
    draw_graph(forecast_24h->precipitation.data(), 24, 10 + 380, 7, 240, 80, 0.0, 100.0, GxEPD_BLACK, 2, 10, 4, 0, 9, true);
}

void draw_24_h_graphs(Weather_24H &forecast_24h, struct tm timeinfo, int day_offset) {

    // Print the day label
    display.setFont(&FreeMonoBold18pt7b);
    display.setTextColor(GxEPD_BLACK);
    char buffer[10];
    timeinfo.tm_mday += day_offset;
    mktime(&timeinfo); // normalize
    strftime(buffer, sizeof(buffer), "%a", &timeinfo);
    display.setCursor(30, 140);
    display.print(buffer);
    float min_temp = min_element(forecast_24h.temperature.data()+day_offset*24, 24);
    float max_temp = max_element(forecast_24h.temperature.data()+day_offset*24, 24);

    // Draw temperature, precipitation, humidity, and cloudcover graphs
    draw_graph(forecast_24h.temperature.data()+day_offset*24, 24, 30, 150, D_WIDTH - 60, D_HEIGHT - 180, min_temp, max_temp, GxEPD_RED, 3, 9, 6, 0, 23,false, 25);
    draw_graph(forecast_24h.precipitation.data()+day_offset*24, 24, 30, 150, D_WIDTH - 60, D_HEIGHT - 180, 0.0, 100.0, GxEPD_BLACK, 3, 10, 4, 0, 23,true);
    draw_graph(forecast_24h.humidity.data()+day_offset*24, 24, 30, 150, D_WIDTH - 60, D_HEIGHT - 180, 0.0, 100.0, GxEPD_RED, 1, 10, 4, 0, 23,true);
    draw_graph(forecast_24h.cloudcover.data()+day_offset*24, 24, 30, 150, D_WIDTH - 60, D_HEIGHT - 180, 0.0, 100.0, GxEPD_BLACK, 1, 10, 4, 0, 23,true);
}

void draw_5_day_graphs(Weather_5D &forecast_5d, struct tm timeinfo, int day_offset) {

    // Print the day label (maybe duplicate code, but if it works don't touch it)
    display.setFont(&FreeMonoBold18pt7b);
    display.setTextColor(GxEPD_BLACK);
    char buffer[10];
    timeinfo.tm_mday += day_offset;
    mktime(&timeinfo); // normalize
    strftime(buffer, sizeof(buffer), "%a", &timeinfo);
    display.setCursor(30, 140);
    display.print(buffer);

    float min_temp = min_element(forecast_5d.temperature.data()+day_offset*24, 120);
    float max_temp = max_element(forecast_5d.temperature.data()+day_offset*24, 120);

    draw_graph(forecast_5d.temperature.data()+day_offset*24, 120, 30, 150, D_WIDTH - 60, D_HEIGHT - 180, min_temp, max_temp, GxEPD_RED, 3, 15, 6, 0, 120,false, 25);
    
    // Draw a line to separate days
    for (int i = 1; i < 5; i++) {
        int x_pos = 30 + i * (float)( (D_WIDTH - 60) / 5);
        display.drawLine(x_pos, 150, x_pos, D_HEIGHT - 30, GxEPD_BLACK);
        timeinfo.tm_mday += 1;
        mktime(&timeinfo); // normalize
        strftime(buffer, sizeof(buffer), "%a", &timeinfo);
        display.setFont(&FreeMonoBold18pt7b);
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(x_pos + 5, 140);
        display.print(buffer);
    }
    draw_graph(forecast_5d.precipitation_probability.data()+day_offset*24, 120, 30, 150, D_WIDTH - 60, D_HEIGHT - 180, 0.0, 100.0, GxEPD_BLACK, 3, 10, 4, 0, 23,true);
    
}

void draw_bus_arrivals(RouteInfo routes[], int num_routes, int shift) {
    
    routes += shift;

    // Draw bus arrivals box
    display.drawRect(10, 115, D_WIDTH - 20, D_HEIGHT - 125, GxEPD_BLACK);
    
    int anc_x = 10;
    int anc_y = 115;
    
    int rout_index = 0;
    while(true){

        // If no more routes have been fetched, break
        if(routes[rout_index].shortName == ""){
            break;
        }
        display.drawRect(anc_x, anc_y, (D_WIDTH - 20)/2, (D_HEIGHT - 124)/5, GxEPD_BLACK);
        display.setFont(&FreeMonoBold24pt7b);
        display.setTextColor(GxEPD_BLACK);

        // Print route number checking if numeric or not
        int num = atoi(routes[rout_index].shortName.c_str());
        display.setCursor((num>9?anc_x+5:anc_x+20), anc_y + 40);
        if(num==0 && routes[rout_index].shortName!="0"){
            //non numeric route
            String shortName = routes[rout_index].shortName;
            display.setCursor((shortName.length()>1?anc_x+5:anc_x+20), anc_y + 40);
            display.print(routes[rout_index].shortName);
        }
        else{
            display.print(num);
        }
        display.setFont(&FreeMonoBold18pt7b);

        int del = routes[rout_index].delay;

        // Calculate adjusted arrival time considering delay
        int val = calc_delta_time(routes[rout_index].eta)+del;
        if(val<6&&val>-1){
            display.setTextColor(GxEPD_RED);
        }
        else{
            display.setTextColor(GxEPD_BLACK);
        }
        display.setCursor(anc_x + ((val<10&&val>-1)?68:58), anc_y + 28);
        if(val>99){
            display.setCursor(anc_x + 63, anc_y + 28);
            display.setFont(&FreeMonoBold9pt7b);
            display.print(val);
        }
        else{
            if(val<-9){
                display.setFont(&FreeMonoBold9pt7b);
            }
            display.print(val);
        }
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(anc_x + 65, anc_y + 43);
        display.print("min");
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(anc_x + 120, anc_y + 37);
        display.print(routes[rout_index].eta);
        
        // Print delay
        if(del<10&&del>-10){
            display.print(" ");
        }
        if(del>0){
            display.setTextColor(GxEPD_RED);
            display.print("+");
            display.print(del);
        }
        else if (del<0){
            display.setTextColor(GxEPD_BLACK);
            display.print(del);
        }
        else{
            display.setTextColor(GxEPD_BLACK);
            display.print("-");
        }
        anc_y += (D_HEIGHT - 124)/5;
        if(anc_y + (D_HEIGHT - 124)/5 > D_HEIGHT - 9){
            anc_y = 115;
            anc_x += (D_WIDTH - 20)/2;
        }
        if(anc_x + (D_WIDTH - 20)/2 > D_WIDTH - 10){
            break;
        }
        rout_index++;

    }
}

void draw_5_day_forecast(Weather_5D &forecast_5d, struct tm &timeinfo, int shift_days) {
    
    int width_scale = 5;

    // Positions for weather icons and data
    int positions[6][2] = {{35, 100},{35+(D_WIDTH-width_scale)/3, 100},{35+2*(D_WIDTH-width_scale)/3, 100},
                            {35, 135+(D_HEIGHT-180)/2},{35+(D_WIDTH-width_scale)/3, 135+(D_HEIGHT-180)/2},{35+2*(D_WIDTH-width_scale)/3, 135+(D_HEIGHT-180)/2}};
    
    // Draw weather icons and data for each of the 6 days
    for (int i = 0; i < 6; i++) {
        int code = forecast_5d.codes[i + shift_days];
        display.drawBitmap(positions[i][0], positions[i][1], epd_bitmap_allArray[weather_codes_to_btm[code]], 150, 150, GxEPD_BLACK);
    }

    // Print day labels, precipitation probabilities, and min/max temperatures
    for (int i = 0; i < 6; i++) {
        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(GxEPD_BLACK);
        char buffer[10];
        struct tm temp_time = timeinfo;
        temp_time.tm_mday += i + shift_days;
        mktime(&temp_time); // normalize
        strftime(buffer, sizeof(buffer), "%a", &temp_time);
        display.setCursor(positions[i][0], positions[i][1]+35);
        display.print(buffer);
        display.setCursor(positions[i][0], positions[i][1] + 55);
        if(forecast_5d.precipitation[i+ shift_days] > 50){
            display.setTextColor(GxEPD_RED);
        } else {
            display.setTextColor(GxEPD_BLACK);
        }
        display.print(forecast_5d.precipitation[i + shift_days]);
        display.print("%");
        display.setTextColor(GxEPD_BLACK);

        // Print min and max temperatures
        float min_temp = forecast_5d.temp_min[i + shift_days];
        float max_temp = forecast_5d.temp_max[i + shift_days];
        display.setCursor(positions[i][0], positions[i][1] + 145);
        display.print(min_temp, 1);
        display.print("C");
        display.print(" - ");
        display.print(max_temp, 1);
        display.print("C");
    }
}

void draw_big_time(struct tm &timeinfo, bool five_min_mode) {
    // Draw big time display
    char buffer[30];
    display.setTextColor(GxEPD_BLACK);
    if(five_min_mode){
        timeinfo.tm_min = (timeinfo.tm_min / 5) * 5; // round down to nearest 5
    }
    mktime(&timeinfo); // normalize
    strftime(buffer, sizeof(buffer), "%H:%M", &timeinfo);
    display.setFont(&FreeMonoBold24pt7b);
    display.setTextSize(4);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((D_WIDTH - w) / 2 - x1, (D_HEIGHT +70 - h) / 2 - y1);
    display.print(buffer);
    display.setTextSize(1);
    
    // Draw icons for various statuses alligned with leds
    int positions[5] = {D_WIDTH - 50+10, D_WIDTH - 160+10, D_WIDTH - 280+10, D_WIDTH - 402+10, D_WIDTH - 523+10};

    for(int i = 0; i < 5; i++){
        display.drawBitmap(positions[i], D_HEIGHT - 30, icons_bitmap_allArray[4-i], 30, 30, GxEPD_BLACK);
    }

}