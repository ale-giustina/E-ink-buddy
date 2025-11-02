#include <Arduino.h>
#include "secrets.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <map>

std::map<int, String> routeMap;

void setup() {

  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  {
    HTTPClient routeClient;
    String url = String(TT_BASE_URL) + "/routes?areas=23";
    routeClient.begin(url);
    routeClient.setAuthorization(TT_USER, TT_PASS);
    int httpCode = routeClient.GET();
    if (httpCode == 200) {
      String payload = routeClient.getString();
      Serial.println(payload);
      JsonDocument doc;
      
      if (!deserializeJson(doc, payload)) {
        
        JsonArray arr = doc.as<JsonArray>();

        for (JsonObject elem : arr) {
          Serial.print(elem["routeId"].as<int>());
          Serial.print(" - ");
          Serial.print(elem["routeShortName"].as<const char*>());
          Serial.print(" - ");
          Serial.println(elem["routeLongName"].as<const char*>());
          int routeId = elem["routeId"].as<int>();
          const char* shortName = elem["routeShortName"];
          if (shortName != nullptr) {
            routeMap[routeId] = String(shortName);
          }
        }
        Serial.printf("Loaded %d routes\n", (int)routeMap.size());
      } else {
        Serial.println("Failed to parse routes JSON");
      }
    } else {
      Serial.printf("Failed to fetch routes, HTTP code: %d\n", httpCode);
    }
    routeClient.end();
  }


}


void loop() {
  vTaskDelete(NULL);
}
