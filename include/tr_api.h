// Author: Alessandro Giustina
// Description: Transit API interaction header file

#ifndef TR_API_H
#define TR_API_H

#include <Arduino.h>
#include "secrets.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <map>
#include <helpers.h>

/**
 * @brief Structure to hold route information.
 * 
 */
struct RouteInfo {
    String shortName; // e.g., "3", "5/", "L1"
    String longName; // e.g., "Cortesano Gardolo P.Dante Villazzano 3"
    int delay; // Delay in minutes
    String eta; // Estimated time of arrival in "HH:MM" format
};

extern bool enable_debug;

// The map that stores route info (routeId → [shortName, longName])
extern std::map<int, std::array<String, 2>> routeMap;

// The maximum number of retries for HTTP requests after a failed json parse
#define MAX_RETRIES 3

// Size of the buffer used for reading HTTP responses (50000 bytes is enough for lengths<=12)
#define BUFFER_SIZE 50000

const int DAYLIGHT_SAVING[] = {3, 10, 31, 3}; //m m d d

/**
 * @brief Fetches all routes from the API and populates routeMap.
 * 
 * This function contacts the Transit API (using TT_BASE_URL, TT_USER, TT_PASS from secrets.h)
 * and fills routeMap with route IDs and their corresponding short/long names.
 * 
 * @return true if the route map was successfully created, false otherwise.
 */
bool create_route_map();

/**
 * @brief Fetch stop info for a specific stop and populate the provided RouteInfo array.
 * 
 * @param stopId The stop ID to query.
 * @param info Pointer to a RouteInfo array that will be filled with the results.
 * @param length Number of trips to fetch.
 * @param shift If not -1, the query time will be shifted by this amount (in minutes, optional).
 * 
 */
void get_stop_info(int stopId, RouteInfo* info, int length, int shift=-1);

/**
 * @brief Fetch stop info for a specific stop and populate the provided RouteInfo array.
 * 
 * @param stopId The stop ID to query.
 * @param info Pointer to a RouteInfo array that will be filled with the results.
 * @param length Number of trips to fetch.
 * @param routeId Only trips matching this route id will be included.
 * @param direction If false, fetch trips in the forward direction; if true, fetch in the reverse direction.
 * @param autoShift If true, automatically adjust the query time to fetch upcoming trips only (default: true).
 * 
 *  396 - 3 - Cortesano Gardolo P.Dante Villazzano 3 &<br>
 *  400 - 5 - Piazza Dante P.Fiera Povo Oltrecastello
 *  402 - 7 - Canova Melta Piazza Dante Gocciadoro
 *  404 - 8 - Centochiavi Piazza Dante Mattarello
 *  406 - 9 - P.Dante S.Donà Cognola Villamontagna
 *  408 - 10 - P.Dante Martignano Cognola / Montevaccino
 *  425 - 11 - Dogana Ftm Via Brennero Gardolo Spini
 *  466 - 13 - P.Dante Rosmini S.Rocco Povo Polo Soc.
 *  478 - 15 - Dogana Ftm Interporto Spini Di Gardolo
 *  484 - 16 - Centochiavi Martignano Cognola Povo
 *  533 - C - Rsa "S.Bart." "Angeli Custodi" Laste
 *  535 - 5/ - P.Dante P.Fiera Università Mesiano Povo
 *  536 - 1 - Ospedale P.Dante Sopramonte
 *  538 - 2 - Piedic. Stazione Osp.S.Chiara Regione Piedic.
 *  539 - 4 - Gardolo Roncafort Stazione V.Deg. Mad.Bianca
 *  541 - 6 - Vela P.Dante Villazzano Grotta
 *  568 - G - Gardolo Gardolo Di Mezzo E Spini
 *  570 - CM - Povo Borino Celva Passo Cimirlo
 *  607 - L1 - 01 Lavis Pressano Nave S.Felice Sorni
 *  610 - L3 - 03 Lavis Stazione Fs
 *  612 - L4 - 04 Lavis Zona Industriale Lavis
 *  613 - L2 - 02 Lavis Pressano Masi Sp.131 Sorni
 *  614 - A - P.Dante Ospedale Fersina Clarina P.Dante
 *  615 - M - Martignano Montevaccino
 *  621 - N - Zuffo Via Rosmini Italcementi
 *  623 - 19 - Stazione Buc M.Baldo Muse Regione
 *  624 - 12 - P.Dante V.Ghiaie Ravina Romagnano
 *  626 - 14 - P.Dante Via Sanseverino Belvedere
 *  629 - 17 - Dogana Ftm V.Bolzano Lavis Lavs Ftm
 * 
 */
void get_stop_info_filtered(int stopId, RouteInfo* info, int length, int routeId, bool direction, bool autoShift=true);



#endif