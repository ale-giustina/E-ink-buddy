#ifndef TR_API_H
#define TR_API_H

#include <Arduino.h>
#include <map>

struct RouteInfo {
    String shortName;
    String longName;
    int delay;
    String eta;
};

// The map that stores route info (routeId → [shortName, longName])
static std::map<int, String[2]> routeMap;


/**
 * @brief Fetches all routes from the API and populates routeMap.
 * 
 * This function contacts the Transit API (using TT_BASE_URL, TT_USER, TT_PASS from secrets.h)
 * and fills routeMap with route IDs and their corresponding short/long names.
 */
void create_route_map();

/**
 * @brief Fetch stop info for a specific stop and populate the provided RouteInfo array.
 * 
 * @param stopId The stop ID to query.
 * @param info Pointer to a RouteInfo array that will be filled with the results.
 * @param length Number of trips to fetch.
 */
void get_stop_info(int stopId, RouteInfo* info, int length);

#endif
