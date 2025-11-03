#include <tr_api.h>


std::map<int, std::array<String, 2>> routeMap;

void create_route_map(){

  HTTPClient routeClient;
  String url = String(TT_BASE_URL) + "/routes?areas=23";
  routeClient.begin(url);
  routeClient.setAuthorization(TT_USER, TT_PASS);
  int httpCode = routeClient.GET();
  if (httpCode == 200) {
    String payload = routeClient.getString();

    JsonDocument doc;
    
    if (!deserializeJson(doc, payload)) {
      
      JsonArray arr = doc.as<JsonArray>();

      for (JsonObject elem : arr) {
        debug_print(elem["routeId"].as<String>());
        debug_print(" - ");
        debug_print(elem["routeShortName"].as<String>());
        debug_print(" - ");
        debug_println(elem["routeLongName"].as<String>());
        
        int routeId = elem["routeId"].as<int>();
        const char* shortName = elem["routeShortName"];
        const char* longName = elem["routeLongName"];
        String info[2] = {String(shortName), String(longName)};
        if (shortName != nullptr) {
          routeMap[routeId][0] = info[0];
          routeMap[routeId][1] = info[1];
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

void get_stop_info(int stopId, RouteInfo *info, int length){

  HTTPClient stopClient;

  String url = String(TT_BASE_URL) + String("/trips_new?stopId=") + String(stopId) + String("&type=U&limit=") + String(length);
  
  stopClient.begin(url);
  stopClient.setAuthorization(TT_USER, TT_PASS);
  int httpCode = stopClient.GET();
  
  if (httpCode == 200) {

    Serial.print("Fetching trips for stop ");
    Serial.println(stopId);
    String payload = stopClient.getString();
    //debug_println(payload);
    JsonDocument doc;
    
    if (!deserializeJson(doc, payload)) {
      
      JsonArray arr = doc.as<JsonArray>();
      for (JsonObject elem : arr) {

        debug_print(String(atoi(routeMap[elem["routeId"].as<int>()][0].c_str())));
        if(length > 0){
          debug_print(" - ");
          debug_print(elem["routeId"].as<String>());
          info->shortName = routeMap[elem["routeId"].as<int>()][0];
          debug_print(" - ");
          debug_print(routeMap[elem["routeId"].as<int>()][0]);
          info->longName = routeMap[elem["routeId"].as<int>()][1];
          debug_print(" - ");
          debug_print(routeMap[elem["routeId"].as<int>()][1]);
          debug_print(" - ");
          debug_print(elem["delay"].as<String>());
          info->delay = elem["delay"].as<int>();
          debug_print(" - ");
          
          String arrivalTime = "";
          int colonIndex = 0;
          bool recording = false;
          for (char c : elem["oraArrivoProgrammataAFermataSelezionata"].as<String>()) {
            if (c == 'T') {
              recording = true;
            } else if (c == ':') {
              if(colonIndex == 1) break;
              colonIndex++;
              arrivalTime += c;
            } else if (recording) {
              arrivalTime += c;
            }
          }

          debug_println(arrivalTime);
          info->eta = arrivalTime;
          
          info++;

          length--;

        }
      }
    } else {
      Serial.println("Failed to parse routes JSON");
      debug_println(payload);
    }
  } else {
    Serial.printf("Failed to fetch routes, HTTP code: %d\n", httpCode);
  }
  stopClient.end();
}

void get_stop_info_filtered(int stopId, RouteInfo *info, int length, int routeId, bool direction){

  HTTPClient stopClient;

  String url = String(TT_BASE_URL) + String("/trips_new?routeId=") + String(routeId) + String("&type=U&limit=") + String(length) + String("&directionId=") + String(direction ? "1" : "0");
  
  stopClient.begin(url);
  stopClient.setAuthorization(TT_USER, TT_PASS);
  int httpCode = stopClient.GET();
  
  if (httpCode == 200) {

    Serial.print("Fetching trips for stop ");
    Serial.println(stopId);
    String payload = stopClient.getString();
    //debug_println(payload);
    JsonDocument doc;
    
    if (!deserializeJson(doc, payload)) {
      
      JsonArray arr = doc.as<JsonArray>();
      for (JsonObject elem : arr) {
        
        for (JsonObject stop : elem["stopTimes"].as<JsonArray>()) {
          if (stop["stopId"].as<int>() == stopId) {

            if(length > 0){
              debug_print(" - ");
              debug_print(elem["routeId"].as<String>());
              info->shortName = routeMap[elem["routeId"].as<int>()][0];
              debug_print(" - ");
              debug_print(routeMap[elem["routeId"].as<int>()][0]);
              info->longName = routeMap[elem["routeId"].as<int>()][1];
              debug_print(" - ");
              debug_print(routeMap[elem["routeId"].as<int>()][1]);
              debug_print(" - ");
              debug_print(elem["delay"].as<String>());
              info->delay = elem["delay"].as<int>();
              debug_print(" - ");

              String arrivalTime = "";
              bool recording = true;
              int colonIndex = 0;
              for (char c : stop["arrivalTime"].as<String>()) {
                if (c == ':') {
                  if (colonIndex == 1) break;
                  colonIndex++;
                  arrivalTime += c;
                } else if (recording) {
                  arrivalTime += c;
                }
              }

              debug_println(arrivalTime);
              info->eta = arrivalTime;
              info++;
              length--;

            }

          }
        }
        
      }
    } else {
      Serial.println("Failed to parse routes JSON");
      debug_println(payload);
    }
  } else {
    Serial.printf("Failed to fetch routes, HTTP code: %d\n", httpCode);
  }
  stopClient.end();
}