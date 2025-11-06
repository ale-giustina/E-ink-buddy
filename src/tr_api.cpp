#include <tr_api.h>


std::map<int, std::array<String, 2>> routeMap;

bool create_route_map(){
  bool success = true;
  //TODO: make this resilient to failed requests

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
      success = true;
    } else {
      Serial.println("Failed to parse routes JSON");
      success = false;
    }
  } else {
    Serial.printf("Failed to fetch routes, HTTP code: %d\n", httpCode);
    success = false;
  }
  routeClient.end();
  return success;
}

void get_stop_info(int stopId, RouteInfo *info, int length, int shift){
  String queryTime = "";
  if(shift != -1){
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    time_t rawtime = mktime(&timeinfo);
    rawtime += shift * 60;
    if(!is_DST(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour)){
      rawtime -= 3600;
    }
    struct tm * shiftedTime = localtime(&rawtime);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", shiftedTime);
    queryTime = String(buffer);
  }
  
  int retries = 0;
  while(retries < MAX_RETRIES){
    HTTPClient stopClient;

    String url = String(TT_BASE_URL) + String("/trips_new?stopId=") + String(stopId) + String("&type=U&limit=") + String(length);
    if(shift != -1) url += String("&refDateTime=") + String(queryTime);
    debug_println("Request URL: " + url);
    stopClient.begin(url);
    stopClient.setAuthorization(TT_USER, TT_PASS);
    int httpCode = stopClient.GET();
    
    if (httpCode == 200) {
      Serial.print("Fetching trips for stop: ");
      Serial.println(stopId);
      String payload = stopClient.getString();

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {

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
                else{
                  int hours = arrivalTime.toInt();
                  struct tm timeinfo;
                  getLocalTime(&timeinfo);
                  if(!is_DST(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour)){
                    debug_print("!DST - ");
                    hours += 1;
                    if(hours >= 24) hours -= 24;
                  }
                  arrivalTime = hours < 10 ? "0" + String(hours) : String(hours);
                }
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
        debug_println("Stop info fetch successful.");
        break; // Exit the retry loop on success
      } else {
        debug_print("Failed to parse routes JSON: ");
        debug_println(error.c_str());
      }
    } else {
      char buffer[20];
      sprintf(buffer, "Failed to fetch routes, HTTP code: %d\n", httpCode);
      debug_print(buffer);
      sprintf(buffer, "URL: %s\n", url.c_str());
      debug_print(buffer);
    }
    stopClient.end();
    retries++;
    if(retries >= MAX_RETRIES){
      debug_println("Max retries reached for get_stop_info_filtered");
      return;
    }
    else{
      debug_println("Retrying get_stop_info...");
    }
  
  }
  Serial.println("get_stop_info successful");
}

void get_stop_info_filtered(int stopId, RouteInfo *info, int length, int routeId, bool direction, bool autoShift){
  int retries = 0;

  String queryTime = "";

  if(autoShift){
    get_stop_info_filtered(stopId, info, length, routeId, direction, false);
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    for(int i=0; i<length; i++){
      
      int etaMinutes = info[i].eta.substring(3,5).toInt();
      int etaHours = info[i].eta.substring(0,2).toInt();

      if(((etaMinutes > timeinfo.tm_min && etaHours >= timeinfo.tm_hour) || (etaHours > timeinfo.tm_hour || etaHours > timeinfo.tm_hour+12)) && (i!=0)){
        queryTime = "";
        queryTime += String(timeinfo.tm_year + 1900) + "-";
        queryTime += (timeinfo.tm_mon + 1 < 10 ? "0" : "") + String(timeinfo.tm_mon + 1) + "-";
        queryTime += (timeinfo.tm_mday < 10 ? "0" : "") + String(timeinfo.tm_mday) + "T";
        int hours = info[i-1].eta.substring(0,2).toInt();
        queryTime += (hours < 10 ? "0" : "") + String(hours) + ":";
        queryTime += (info[i-1].eta.substring(3,5).toInt() < 10 ? "0" : "") + String(info[i-1].eta.substring(3,5).toInt()) + ":";
        queryTime += String("00") + "Z";
        debug_println("Auto-shifting query time to: " + queryTime);
        break;
      }
      else if(i==length-1){
        //If we reach the end without finding a suitable time, set queryTime to now + 1 hour
        time_t rawtime = mktime(&timeinfo);
        rawtime += 3600;
        struct tm * shiftedTime = localtime(&rawtime);
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", shiftedTime);
        queryTime = String(buffer);
        debug_println("Couldn't find suitable stop\nAuto-shifting query time to: " + queryTime);
      }
    }

  }
  while(retries < MAX_RETRIES){

    HTTPClient stopClient;
    String url = String(TT_BASE_URL) + String("/trips_new?routeId=") + String(routeId) + String("&type=U&limit=") + String(length / 2) + String("&directionId=") + String(direction ? "1" : "0");
    if(queryTime != "") url += String("&refDateTime=") + String(queryTime);
    debug_println("Request URL: " + url);
    stopClient.begin(url);
    stopClient.setAuthorization(TT_USER, TT_PASS);
    int httpCode = stopClient.GET();
    
    if (httpCode == 200) {
      Serial.print("Fetching filtered trips for stop: ");
      Serial.println(stopId);
      //String payload = stopClient.getString();

      String payload = stopClient.getString();

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {

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
                    if (colonIndex == 1) {
                      break;
                    } else {
                      int hours = arrivalTime.toInt();
                      arrivalTime = hours < 10 ? "0" + String(hours) : String(hours);
                    }
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
        debug_println("Filtered stop info fetch successful.");
        break; // Exit the retry loop on success
      } else {
        debug_print("Failed to parse routes JSON: ");
        debug_println(error.c_str());
      }
    } else {
      char buffer[50];
      sprintf(buffer, "Failed to fetch routes, HTTP code: %d\n", httpCode);
      debug_print(buffer);
    }
    stopClient.end();
    retries++;
    if(retries >= MAX_RETRIES){
      debug_println("Max retries reached for get_stop_info_filtered");
      return;
    }
    else{
      debug_println("Retrying get_stop_info_filtered...");
    }
  }
  if(autoShift) Serial.println("get_stop_info_filtered successful");
}


/*
Deprecated stream based approach
void get_stop_info_filtered(int stopId, RouteInfo *info, int length, int routeId, bool direction){
  int retries = 0;
  while(retries < MAX_RETRIES){

    HTTPClient stopClient;
    String url = String(TT_BASE_URL) + String("/trips_new?routeId=") + String(routeId) + String("&type=U&limit=") + String(length) + String("&directionId=") + String(direction ? "1" : "0");
    
    stopClient.begin(url);
    stopClient.setAuthorization(TT_USER, TT_PASS);
    int httpCode = stopClient.GET();
    
    if (httpCode == 200) {
      Serial.print("Fetching filtered trips for stop ");
      Serial.println(stopId);

      WiFiClient& stream = stopClient.getStream();

      while (stream.available()) {
        char buf[64];
        stream.readBytesUntil(':', buf, sizeof(buf));
        buf[sizeof(buf) - 1] = '\0';  // Ensure null-termination
        if(strstr(buf, "stopId") != NULL){
          memset(buf, 0, sizeof(buf));
          stream.readBytesUntil(',', buf, (size_t)20);
          buf[sizeof(buf) - 1] = '\0';  // Ensure null-termination
          if(atoi(buf)==stopId){
            Serial.print("Found stopId field: ");
            Serial.println(buf);
            memset(buf, 0, sizeof(buf));
            stream.readBytesUntil(',', buf, (size_t)20);
            buf[sizeof(buf) - 1] = '\0';  // Ensure null-termination
            Serial.print("Reading arrivalTime: ");
            Serial.println(buf);
          }
        }
      }
      break;
    }
    else {
      Serial.printf("Failed to fetch routes, HTTP code: %d\n", httpCode);
    }
    stopClient.end();
    retries++;
    if(retries >= MAX_RETRIES){
      Serial.println("Max retries reached for get_stop_info_filtered");
    }
    else{
      Serial.println("Retrying get_stop_info_filtered...");
    }
  }
}
*/