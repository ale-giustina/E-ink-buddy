#ifndef WEATHER_H
#define WEATHER_H
#include <map>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <time.h>
#include <helpers.h>
#include <HTTPClient.h>

extern std::map<int, String> weather_codes;

struct Weather_5D {

    struct tm last_update = {-1};
    std::array<short, 5> codes;
    std::array<float, 5> temp_min;
    std::array<float, 5> temp_max;
    std::array<short, 5> precipitation;
    
};

//the expiration time in minutes of the 5D weather packet
#define WEATHER_EXP 10


/**
*
*    @brief Fills the weather struct with data from the next 5 days,
*    if the weather isn't older than 10 minutes the force_update flag is needed
*    @param &w_ob a Weather_5D struct to be filled
*    @param force_update Forces the update even if before than 10 minutes (optional, default false)
*
*/
void get_weather_5d(Weather_5D &w_ob, bool force_update=false);

#endif