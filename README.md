# TODO:

## Modules
- [X] Create transport API module to handle arrivals data fetching
- [X] Filter data based on bus
- [X] Create weather API module that supports multiple modes of display (24h, 5d, precipitations...) using open-weather
- [X] 5 Days weather display
- [X] 24 weather Hours display
- [X] Temperature and precipitation graph (?)
- [X] Create timing module to sync every X minutes over NTP
- [X] Resilient wifi error handler
- [X] Create task manager to call respective processes to enable everything to run on time
- [X] BUG: find cause of json incomplete inputs (may be linked)
- [X] Create renderer module to manage the E-ink display
- [X] Optimize buffer usage to fit more bus lines
- [X] Create 5 day forecast drawing
- [X] Create bus arrivals drawing
- [X] Create 5 day forecast graphs drawing
- [X] Handle buttons and state switching between: weather types, lighting sequences and bus filter modes
- [ ] Update README and add schematics and images
## Data managing
The main modules are 
- ### tr_api.h 
    Handles transport API data fetching and parsing
- ### weather.h
    Handles weather API data fetching and parsing
- ### graphics.h
    Handles all drawing functions for the e-ink display
- ### helpers.h
    Contains utility functions for debug and data manipulation tasks.

### Transport API
The transport API module fetches real-time bus arrival data from the trentino transport API. It allows filtering based on specific bus lines and stops. The data is parsed and stored in a structured format for easy access.

### Weather API
The weather API module fetches weather data from the open-meteo API. It supports multiple fetch modes, including 24-hour, 5-day forecasts and current weather fetching. The data is parsed and stored in a structured format for easy access.

### Graphics Module
The graphics module manages all drawing functions for the e-ink display. It includes functions to render weather forecasts, bus arrival times(in progress), and graphical representations such as temperature and precipitation graphs.

Bitmaps taken from [weather-icons](https://github.com/erikflowers/weather-icons/tree/master?tab=readme-ov-file)