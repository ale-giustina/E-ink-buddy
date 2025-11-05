#include <helpers.h>

// Returns formatted part of the local time
String time_helper(char c) {
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

// Prints timestamp in "YYYY-MM-DD HH:MM:SS - " format
void print_timestamp() {
    String timestamp = String(time_helper('Y')) + "-" + String(time_helper('m')) + "-" + String(time_helper('d')) + " " +
                       String(time_helper('H')) + ":" + String(time_helper('M')) + ":" + String(time_helper('S')) + " - ";
    Serial.print(timestamp);
}

bool enable_debug = false;

void debug_println(const String& message) {
    if (enable_debug) {
        Serial.println(message);
    }
}
void debug_print(const String& message) {
    if (enable_debug) {
        Serial.print(message);
    }
}

void debug_print_routes(const RouteInfo* routes, int count) {
    if (enable_debug) {
        for(int i = 0; i < 5; i++) {
            print_timestamp();
            Serial.print("Route: ");
            Serial.print(routes[i].shortName);
            Serial.print(", ");
            Serial.print(routes[i].longName);
            Serial.print(", Delay: ");
            Serial.print(routes[i].delay);
            Serial.print(" min, ETA: ");
            Serial.println(routes[i].eta);
        }
    }
}

void debug_print_weather_5d(const Weather_5D& weather) {
    if (enable_debug) {
        print_timestamp();
        Serial.println("5-Day Weather Forecast:");
        for (int i = 0; i < 5; i++) {
            Serial.print("Day ");
            Serial.print(i + 1);
            Serial.print(": Code=");
            Serial.print(weather.codes[i]);
            Serial.print(", Temp Min=");
            Serial.print(weather.temp_min[i]);
            Serial.print(", Temp Max=");
            Serial.print(weather.temp_max[i]);
            Serial.print(", Precipitation Probability=");
            Serial.println(weather.precipitation[i]);
        }
    }
}

void debug_print_weather_24h(const Weather_24H& weather) {
    if (enable_debug) {
        print_timestamp();
        Serial.println("24-Hour Weather Forecast:");
        Serial.print("Code=");
        Serial.print(weather.code);
        Serial.print(", Temp Min=");
        Serial.print(weather.temp_min);
        Serial.print(", Temp Max=");
        Serial.println(weather.temp_max);
        Serial.println("Hourly Precipitation Probability:");
        for (int i = 0; i < 24; i++) {
            Serial.print(weather.precipitation[i]);
            Serial.print("% - ");
        }
        Serial.println();
        Serial.println("Hourly Temperature:");
        for (int i = 0; i < 24; i++) {
            Serial.print(weather.temperature[i]);
            Serial.print("C° - ");
        }
        Serial.println();
    }
}

void debug_print_weather_now(const Weather_now& weather) {
    if (enable_debug) {
        print_timestamp();
        Serial.println("Current Weather:");
        Serial.print("Code=");
        Serial.print(weather.code);
        Serial.print(", Temp=");
        Serial.print(weather.temp);
        Serial.print(", Windspeed=");
        Serial.print(weather.windspeed);
        Serial.print(", Precipitation Probability=");
        Serial.print(weather.precipitation_probability);
        Serial.print(", Humidity=");
        Serial.println(weather.humidity);
    }
}