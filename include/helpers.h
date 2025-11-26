// Author: Alessandro Giustina
// Description: Utility helper functions and debugging tools

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

/**
 * @brief Helper function to format time components as strings.
 * 
 * @param c The time component character ('H' for hours, 'M' for minutes, 'S' for seconds).
 * @return Formatted time component as a String.
 */
String time_helper(char c);

/**
 * @brief Print the current timestamp for debugging.
 */
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

/**
 * @brief Calculate the delta time in minutes from the current time to the given ETA string.
 * 
 * @param eta_str The ETA string in "HH:MM" format.
 * @return The delta time in minutes.
 */
int calc_delta_time(const String& eta_str);

/**
 * @brief Returns max element in array
 * 
 * @param arr The array to search.
 * @param len The length of the array.
 * @return The maximum element in the array.
 */
template<typename M>
M max_element(M arr[], int len) {
    M max_val = arr[0];
    for (int i = 1; i < len; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
    }
    return max_val;
}

/**
 * @brief Returns min element in array
 * 
 * @param arr The array to search.
 * @param len The length of the array.
 * @return The minimum element in the array.
 */
template<typename m>
m min_element(m arr[], int len) {
    m min_val = arr[0];
    for (int i = 1; i < len; i++) {
        if (arr[i] < min_val) {
            min_val = arr[i];
        }
    }
    return min_val;
}

#endif // UTILS_H
