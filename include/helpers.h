#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <time.h>
#include <tr_api.h>
#include <weather.h>

struct RouteInfo;
struct Weather_5D;
struct Weather_24H;
struct Weather_now;

// Returns formatted part of the local time
String time_helper(char c);

// Prints timestamp in "YYYY-MM-DD HH:MM:SS - " format
void print_timestamp();

extern bool enable_debug;

/**
 * @brief Print debug message with newline if debugging is enabled.
 * 
 * @param message The message to print.
**/
void debug_println(const String& message);

/**
 * @brief Print debug message if debugging is enabled.
 * 
 * @param message The message to print.
**/
void debug_print(const String& message);

/**
 * @brief Print the contents of a RouteInfo array for debugging.
 * 
 * @param routes Pointer to the RouteInfo array.
 * @param count Number of RouteInfo entries in the array.
 *
**/
void debug_print_routes(const RouteInfo* routes, int count);

/**
 * @brief Print the contents of a Weather_5D struct for debugging.
 * 
 * @param weather The Weather_5D struct to print.
 *
**/
void debug_print_weather_5d(const Weather_5D& weather);

/**
 * @brief Print the contents of a Weather_24H struct for debugging.
 * 
 * @param weather The Weather_24H struct to print.
 * 
**/
void debug_print_weather_24h(const Weather_24H& weather);

/**
 * @brief Print the contents of a Weather_now struct for debugging.
 * 
 * @param weather The Weather_now struct to print.
 *
**/
void debug_print_weather_now(const Weather_now& weather);

/**
 * @brief Check if the device is connected to the internet.
 * 
 * @return true if connected, false otherwise.
 */
bool is_connected();

#endif // UTILS_H
