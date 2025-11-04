#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <time.h>
#include <tr_api.h>
#include <weather.h>

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

#endif // UTILS_H
