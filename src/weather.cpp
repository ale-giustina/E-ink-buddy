#include <weather.h>

std::map<int, String> weather_codes = {
    {0,  "Clear sky"},
    {1,  "Mainly clear"},
    {2,  "Partly cloudy"},
    {3,  "Overcast"},
    {45, "Fog"},
    {48, "Depositing rime fog"},
    {51, "Drizzle: Light intensity"},
    {53, "Drizzle: Moderate intensity"},
    {55, "Drizzle: Dense intensity"},
    {56, "Freezing Drizzle: Light intensity"},
    {57, "Freezing Drizzle: Dense intensity"},
    {61, "Rain: Slight intensity"},
    {63, "Rain: Moderate intensity"},
    {65, "Rain: Heavy intensity"},
    {66, "Freezing Rain: Light intensity"},
    {67, "Freezing Rain: Heavy intensity"},
    {71, "Snow fall: Slight intensity"},
    {73, "Snow fall: Moderate intensity"},
    {75, "Snow fall: Heavy intensity"},
    {77, "Snow grains"},
    {80, "Rain showers: Slight"},
    {81, "Rain showers: Moderate"},
    {82, "Rain showers: Violent"},
    {85, "Snow showers: Slight"},
    {86, "Snow showers: Heavy"},
    {95, "Thunderstorm: Slight or moderate"},
    {96, "Thunderstorm with slight hail"},
    {99, "Thunderstorm with heavy hail"}
};


void get_weather_5d(Weather_5D &w_ob, bool force_update){

    struct tm now;

    getLocalTime(&now);

    if(w_ob.last_update.tm_sec==-1 || w_ob.last_update.tm_min<now.tm_min-WEATHER_EXP || w_ob.last_update.tm_hour!=now.tm_hour || force_update){

        debug_println("Updating...");

        HTTPClient weatherClient;

        String url = "https://api.open-meteo.com/v1/forecast?latitude=52.52&longitude=13.41&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max&models=best_match&timezone=Europe%2FBerlin";

        weatherClient.begin(url);
        int httpCode = weatherClient.GET();
        debug_println(String("Code: ")+String(httpCode));
        if (httpCode == 200) {
            
            String payload = weatherClient.getString();
            //Serial.println(payload);
            JsonDocument doc;
            
            if (!deserializeJson(doc, payload)) {
            
                JsonObject root = doc.as<JsonObject>();

                JsonObject daily = root["daily"];
                int inx = 0;

                getLocalTime(&w_ob.last_update);

                for (JsonVariant v : daily["weather_code"].as<JsonArray>()) {
                    w_ob.codes[inx++] = v.as<short>();
                    if(inx==5)break;
                }
                inx = 0;
                for (JsonVariant v : daily["temperature_2m_max"].as<JsonArray>()) {
                    w_ob.temp_max[inx++] = v.as<float>();
                    if(inx==5)break;
                }
                inx = 0;
                for (JsonVariant v : daily["temperature_2m_min"].as<JsonArray>()) {
                    w_ob.temp_min[inx++] = v.as<float>();
                    if(inx==5)break;
                }
                inx = 0;
                for (JsonVariant v : daily["precipitation_probability_max"].as<JsonArray>()) {
                    w_ob.precipitation[inx++] = v.as<short>();
                    if(inx==5)break;
                }

            } else {
            Serial.println("Failed to parse routes JSON");
            debug_println(payload);
            }
        } else {
            Serial.printf("Failed to fetch weather, HTTP code: %d\n", httpCode);
        }
        weatherClient.end();
    }
    else{
        debug_println(String("Using cache"));
    }

}