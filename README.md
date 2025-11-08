# TODO:

## Modules
- [X] Create transport API module to handle arrivals data fetching
- [X] Filter data based on bus
- [X] Create weather API module that supports multiple modes of display (24h, 5d, precipitations...) using open-weather
- [X] 5 Days weather display
- [X] 24 weather Hours display
- [ ] Temperature and precipitation graph (?)
- [X] Create timing module to sync every X minutes over NTP
- [X] Resilient wifi error handler
- [X] Create task manager to call respective processes to enable everything to run on time
- [X] BUG: find cause of json incomplete inputs (may be linked)
- [ ] Create renderer module to manage the E-ink display
- [ ] Handle buttons and state switching between: weather types, lighting sequences and bus filter modes

## Data managing
The main modules are: tr_api and weather_api. They are responsible for fetching and parsing data from the respective APIs. And storing them in a structured way for later use by the renderer module.

### Transport API
The transport API module fetches real-time bus arrival data from a public transport API. It allows filtering based on specific bus lines and stops. The data is parsed and stored in a structured format for easy access.

### Weather API
The weather API module fetches weather data from the open-meteo API. It supports multiple display modes, including 24-hour forecasts and 5-day forecasts. The data is parsed and stored in a structured format for easy access.