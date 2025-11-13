#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <bitmaps.h>
#include <weather.h>
#include <helpers.h>
#include <tr_api.h>

/**
 * @brief Initializes the graphics display.
 * 
 */
void start_graphics();

extern GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT/4> display;

extern const int D_WIDTH;
extern const int D_HEIGHT;

/**
 * @brief Draws a graph on the display.
 * @tparam T The data type of the values to plot (e.g., float, int).
 * @param data Array of data points to plot.
 * @param len Number of data points in the array.
 * @param x X-coordinate of the top-left corner of the graph area.
 * @param y Y-coordinate of the top-left corner of the graph area.
 * @param w Width of the graph area.
 * @param h Height of the graph area.
 * @param min_val Minimum value for the Y-axis.
 * @param max_val Maximum value for the Y-axis.
 * @param color Color of the graph line.
 * @param thickness Thickness of the graph line (default is 3).
 * @param x_ticks Number of ticks on the X-axis (default is 5).
 * @param y_ticks Number of ticks on the Y-axis (default is 5).
 * @param x_tick_label_min Minimum label value for the X-axis ticks (default is 0).
 * @param x_tick_label_max Maximum label value for the X-axis ticks (default is 10).
 * @param noaxis If true, do not draw the axis (default is false).
 * @param x_cyclic X-axis labels will wrap around at this value.
 */
template<typename T>
void draw_graph(T data[], int len, int x, int y, int w, int h, float min_val, float max_val, uint16_t color, int thickness=3, int x_ticks=5, int y_ticks=5, int x_tick_label_min=0, int x_tick_label_max=10, bool noaxis=false, int x_cyclic=0);

/**
 * @brief Draws a time and weather information strip on the display.
 * 
 * @param x X-coordinate of the top-left corner of the strip.
 * @param y Y-coordinate of the top-left corner of the strip.
 * @param timeinfo Local time information to display.
 * @param current_weather Current weather information to display.
 * @param forecast_24h Optional pointer to 24-hour weather forecast data.
 * @param five_min_mode If true, rounds time to the nearest 5 minutes for display.
 * 
 */
void draw_time_strip(int x, int y, struct tm &timeinfo, Weather_now &current_weather, Weather_24H *forecast_24h=NULL, bool five_min_mode=false);

/**
 * @brief Draws 24-hour graphs for temperature, precipitation, humidity, and cloud cover.
 * 
 * @param forecast_24h Reference to the 24-hour weather forecast data.
 * @param timeinfo Local time information for labeling the graphs.
 * @param day_offset Day offset to start the graphs from (default is 1).
 */
void draw_24_h_graphs(Weather_24H &forecast_24h, tm timeinfo, int day_offset=1);

/**
 * @brief Draws 5-day weather forecast including temperature and precipitation graphs.
 * 
 * @param forecast_5d Reference to the 5-day weather forecast data.
 * @param timeinfo Local time information for labeling the graphs.
 * @param shift_days Number of days to shift the forecast display (default is 0).
 */
void draw_5_day_forecast(Weather_5D &forecast_5d, tm &timeinfo, int shift_days=0);

/**
 * @brief Draws 5-day weather graphs for temperature and precipitation.
 * 
 * @param forecast_5d Reference to the 5-day weather forecast data.
 * @param timeinfo Local time information for labeling the graphs.
 * @param day_offset Day offset to start the graphs from (default is 1).
 */
void draw_5_day_graphs(Weather_5D &forecast_5d, tm timeinfo, int day_offset=1);

/**
 * @brief Draws bus arrival information on the display.
 * 
 * @param routes Reference to an array of RouteInfo structures containing bus route information.
 * @param num_routes Number of routes in the array.
 * @param shift Number of routes to shift the display by (default is 0).
 */
void draw_bus_arrivals(RouteInfo routes[], int num_routes, int shift=0);

/**
 * @brief Draws a large time display on the screen.
 * 
 * @param timeinfo Local time information to display.
 * @param five_min_mode If true, rounds time to the nearest 5 minutes for display.
 */
void draw_big_time(struct tm &timeinfo, bool five_min_mode=false);

#endif