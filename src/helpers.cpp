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