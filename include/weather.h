#ifndef WEATHER_H
#define WEATHER_H
#include <map>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <time.h>
#include <helpers.h>
#include <HTTPClient.h>

extern std::map<int, String> weather_codes; // Map of weather codes to descriptions

/**
 * @brief Structure to hold 5-day weather forecast data.
 */
struct Weather_5D {

    struct tm last_update = {-1};   // Time of the last update
    std::array<short, 14> codes;  // Weather codes for each day of the next 14 days
    std::array<float, 14> temp_min; // Minimum temperatures for each day of the next 14 days
    std::array<float, 14> temp_max; // Maximum temperatures for each day of the next 14 days
    std::array<short, 14> precipitation; // Precipitation probabilities for each day of the next 14 days
    std::array<float, 336> temperature; // Hourly temperatures for the next 14 days
    std::array<short, 336> precipitation_probability; // Hourly precipitation probabilities for the next 14 days
    
};

/**
 * @brief Structure to hold 24-hour weather forecast data.
 */
struct Weather_24H {

    struct tm last_update = {-1};   // Time of the last update
    short code; // Overall weather code for the next 24 hours
    float temp_min; // Minimum temperature for the next 24 hours
    float temp_max; // Maximum temperature for the next 24 hours
    std::array<short, 168> precipitation;   // Hourly precipitation probabilities for the next 7 days
    std::array<float, 168> temperature; // Hourly temperatures for the next 7 days
    std::array<short, 168> humidity; // Hourly humidity for the next 7 days
    std::array<float, 168> cloudcover; // Hourly cloud cover for the next 7 days
    
};

/**
 * @brief Structure to hold current weather data.
 */
struct Weather_now {

    struct tm last_update = {-1};   // Time of the last update
    short code; // Current weather code
    float temp; // Current temperature
    float windspeed; // Current wind speed
    short precipitation_probability; // Current precipitation probability
    short humidity; // Current humidity
    
};

// The expiration time in minutes of the 5D weather packet
#define WEATHER_EXP_5D (20)

// The expiration time in minutes of the 24H weather packet
#define WEATHER_EXP_24H (10)

// The expiration time in minutes of the current weather packet
#define WEATHER_EXP_NOW (10)

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