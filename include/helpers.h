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
 * @param override_debug If true, print the message regardless of the global debug setting.
**/
void debug_println(const String& message, bool override_debug=false);

/**
 * @brief Print debug message if debugging is enabled.
 * 
 * @param message The message to print.
 * @param override_debug If true, print the message regardless of the global debug setting.
**/
void debug_print(const String& message, bool override_debug=false);

/**
 * @brief Print the contents of a RouteInfo array for debugging.
 * 
 * @param routes Pointer to the RouteInfo array.
 * @param count Number of RouteInfo entries in the array.
 * @param override_debug If true, print the routes regardless of the global debug setting.
 *
**/
void debug_print_routes(const RouteInfo* routes, int count, bool override_debug=false);

/**
 * @brief Print the contents of a Weather_5D struct for debugging.
 * 
 * @param weather The Weather_5D struct to print.
 * @param override_debug If true, print the weather regardless of the global debug setting.
 *
**/
void debug_print_weather_5d(const Weather_5D& weather, bool override_debug=false);

/**
 * @brief Print the contents of a Weather_24H struct for debugging.
 * 
 * @param weather The Weather_24H struct to print.
 * @param override_debug If true, print the weather regardless of the global debug setting.
 * 
**/
void debug_print_weather_24h(const Weather_24H& weather, bool override_debug=false);

/**
 * @brief Print the contents of a Weather_now struct for debugging.
 * 
 * @param weather The Weather_now struct to print.
 * @param override_debug If true, print the weather regardless of the global debug setting.
 *
**/
void debug_print_weather_now(const Weather_now& weather, bool override_debug=false);

/**
 * @brief Check if the device is connected to the internet.
 * 
 * @return true if connected, false otherwise.
 */
bool is_connected();

/**
 * @brief Determine if the current date/time is in Daylight Saving Time (DST).
 * 
 * @param year The current year.
 * @param month The current month (1-12).
 * @param day The current day of the month (1-31).
 * @param hour The current hour (0-23).
 * @return true if in DST, false otherwise.
 */
bool is_DST(int year, int month, int day, int hour);

#endif // UTILS_H
