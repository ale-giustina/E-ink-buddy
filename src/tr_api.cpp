#include <tr_api.h>


std::map<int, std::array<String, 2>> routeMap;  // Global route map


static char buffer[BUFFER_SIZE]; // Global buffer for reading HTTP stream data

/**
 * @brief Reads the HTTPClient stream into a buffer, handling chunked transfer encoding.
 * @param client The HTTPClient instance to read from.
 * @param ignore_lists If true, ignores JSON lists in the stream. (used in the unfiltered mode)
 * @return The number of bytes read into the buffer.
 * 
 * This function reads data from the HTTPClient's stream, handling chunked transfer encoding.
 * It populates a global buffer with the received data and returns the total number of bytes read.
 * This is needed as the incoming data stream is sent in chunks formatted as <size>\r\n<data>\r\n.
 * The function continues reading until it encounters a chunk size of 0, indicating the end of the stream.
 */
int read_stream_to_buffer(HTTPClient &client, bool ignore_lists=false){
  
  WiFiClient& stream = client.getStream();
      
  char siz_buf[10];
  int index_buff = 0;
  int index_siz = 0;
  bool isPayload = false;
  char c;
  bool isList = false;
  while(true){
    
    // Wait until data is available
    while(!stream.available()){
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    
    c = stream.read();
    if(c == '\r') continue;
    if(c == EOF) break;
    // Every chunk starts with a size line starting and ending with \n
    if(c == '\n'){
      isPayload = !isPayload;
      continue;
    }

    // Look for lists
    if(index_buff>10){
      if(ignore_lists && c == '[') isList = true;
      if(ignore_lists && c == ']' && isList) {
        isList = false;
        buffer[index_buff++] = '[';
      }
      if(ignore_lists && isList) continue;

    }

    // If the chunk size is zero, the stream has ended
    if(!isPayload){
      siz_buf[index_siz++] = c;
      if(siz_buf[0] == '0'){
        break;
      }
    }
    else{ // Else write in buffer
      buffer[index_buff++] = c;
      index_siz = 0;
    }
    
    // Clean up unwanted fields to reduce buffer size

    // ignore "tripId":"0004373042025091020260610"
    if(memcmp(buffer + index_buff - 36, "\"tripId\":", 9) == 0) {
      index_buff -= 37;
    }

    //"oraArrivoEffettivaAFermataSelezionata": null,
    //"oraArrivoProgrammataAFermataSelezionata": null,
    //"corsaPiuVicinaADataRiferimento": false,
    if (index_buff >= 44 && memcmp(buffer + index_buff - 44, "\"oraArrivoEffettivaAFermataSelezionata\":null,", 44) == 0) {
      index_buff -= 45;
    }

    if (index_buff >= 50 && memcmp(buffer + index_buff - 46, "\"oraArrivoProgrammataAFermataSelezionata\":null,", 46) == 0) {
      index_buff -= 47;
    }

    if (index_buff >= 40 && memcmp(buffer + index_buff - 38, "\"corsaPiuVicinaADataRiferimento\":false,", 38) == 0) {
      index_buff -= 39;
    }

    // "departureTime":"XX:XX:XX",
    if (index_buff >= 28 && memcmp(buffer + index_buff - 28, "\"departureTime\":", 16) == 0) {
      index_buff -= 27;
    }

    // "type":"U",
    if (index_buff >= 28 && memcmp(buffer + index_buff - 10, "\"type\":\"U\",", 10) == 0) {
      index_buff -= 11;
    }

    //stopSequenc
    if (index_buff >= 20 && memcmp(buffer + index_buff - 10, "stopSequenc", 10) == 0) {
      index_buff -= 10;
    }

    // Check for buffer overflow
    if(index_buff >= BUFFER_SIZE-1){
      debug_println("Buffer overflow!");
      break;
    }
  };
  // Null-terminate the buffer and return the size
  buffer[index_buff] = '\0';
  return index_buff;
}

// Create route map from API to map ID to names
bool create_route_map(){

  bool success = true;

  HTTPClient routeClient;
  String url = String(TT_BASE_URL) + "/routes?areas=23"; // Routes in Trento area
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
  
  // Prepare query time if shift is specified
  String queryTime = "";
  if(shift != -1){
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    time_t rawtime = mktime(&timeinfo);
    rawtime += shift * 60;

    // Adjust for DST, this is needed ONLY for the oraArrivoProgrammataAFermataSelezionata field as it returns local time without DST adjustment (i think)
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

      int index_buff = read_stream_to_buffer(stopClient, true);

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, buffer);
      if (!error) {

        JsonArray arr = doc.as<JsonArray>();
        for (JsonObject elem : arr) {

          debug_print(String(atoi(routeMap[elem["routeId"].as<int>()][0].c_str())));
          if(length > 0){

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
            
            // Reformatting arrival time to HH:MM and adjusting for DST
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
        debug_println("Payload size: " + String(index_buff) + " bytes");
        // Exit retry loop on success
        break;
      } else {
        debug_print("Failed to parse routes JSON: ");
        debug_println(error.c_str());
        for(int i=0; i<index_buff; i++){
          debug_print(String(buffer[i]));
        }
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

    // Fetch current stop info to determine next suitable time as reference
    get_stop_info_filtered(stopId, info, length, routeId, direction, false);
    struct tm timeinfo;
    getLocalTime(&timeinfo);

    for(int i=0; i<length; i++){
      
      int etaMinutes = info[i].eta.substring(3,5).toInt();
      int etaHours = info[i].eta.substring(0,2).toInt();

      // Check if the ETA is in the future
      if(((etaMinutes > timeinfo.tm_min && etaHours >= timeinfo.tm_hour) || (etaHours > timeinfo.tm_hour || etaHours > timeinfo.tm_hour+12)) && (i!=0)){
        queryTime = "";
        queryTime += String(timeinfo.tm_year + 1900) + "-";
        queryTime += (timeinfo.tm_mon + 1 < 10 ? "0" : "") + String(timeinfo.tm_mon + 1) + "-";
        queryTime += (timeinfo.tm_mday < 10 ? "0" : "") + String(timeinfo.tm_mday) + "T";
        
        // Set the shift to the previous entry's ETA time
        int hours = info[i-1].eta.substring(0,2).toInt();
        queryTime += (hours < 10 ? "0" : "") + String(hours) + ":";
        queryTime += (info[i-1].eta.substring(3,5).toInt() < 10 ? "0" : "") + String(info[i-1].eta.substring(3,5).toInt()) + ":";
        queryTime += String("00") + "Z";
        debug_println("Auto-shifting query time to: " + queryTime);
        break;
      }
      else if(i==length-1){
        debug_println("Couldn't find suitable stop.");
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

      int index_buff = read_stream_to_buffer(stopClient);

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, buffer);
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

                // Reformatting arrival time to HH:MM
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
        debug_println("Payload size: " + String(index_buff) + " bytes");
        // Exit retry loop on success
        break;

      } else {
        debug_print("Failed to parse routes JSON: ");
        debug_println(error.c_str());
        Serial.println(buffer);
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