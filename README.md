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
All data between modules should be handled as structs or array of structs.
E.g. the data for busses when the function is called must return an array of structs built as: {short_name, long_name, ETA, delay} (not final, tbd).
<br>The modules will be divided each into their own file. 