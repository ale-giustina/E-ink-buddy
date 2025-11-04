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

struct Weather_24H {

    struct tm last_update = {-1};
    short code;
    float temp_min;
    float temp_max;
    std::array<short, 24> precipitation;
    std::array<float, 24> temperature;
    
};

struct Weather_now {

    struct tm last_update = {-1};
    short code;
    float temp;
    float windspeed;
    short precipitation_probability;
    short humidity;
    
};

//the expiration time in minutes of the 5D weather packet
#define WEATHER_EXP_5D 10

//the expiration time in minutes of the 24H weather packet
#define WEATHER_EXP_24H 1

//the expiration time in minutes of the current weather packet
#define WEATHER_EXP_NOW 1

/**
*
*    @brief Fills the weather struct with data from the next 5 days,
*    if the weather isn't older than 10 minutes the force_update flag is needed
*    @param &w_ob a Weather_5D struct to be filled
*    @param force_update Forces the update even if before than 10 minutes (optional, default false)
*
*/
void get_weather_5d(Weather_5D &w_ob, bool force_update=false);

/**
*
*    @brief Fills the weather struct with data from the next 24 hours,
*    if the weather isn't older than 1 minute the force_update flag is needed to force an api call
*    @param &w_ob a Weather_24H struct to be filled
*    @param force_update Forces the update even if before than 1 minute (optional, default false)
*
*/
void get_weather_24h(Weather_24H &w_ob, bool force_update=false);

/**
*
*    @brief Fills the weather struct with current weather data,
*    if the weather isn't older than 1 minute the force_update flag is needed to force an api call
*    @param &w_ob a Weather_now struct to be filled
*    @param force_update Forces the update even if before than 1 minute (optional, default false)
*
*/
void get_current_weather(Weather_now &w_ob, bool force_update = false);

#endif