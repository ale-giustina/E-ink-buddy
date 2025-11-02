# TODO:

## Modules
- [X] Create transport API module to handle arrivals data fetching
- [ ] Filter data based on bus (in progress)
- [ ] Create weather API module that supports multiple modes of display (24h, 5d, precipitations...) using open-weather
- [ ] Create timing module to sync every X minutes over NTP
- [ ] Create task manager to call respective processes to enable everything to run on time
- [ ] Create renderer module to manage the E-ink display
- [ ] Handle buttons and state switching between: weather types, lighting sequences and bus filter modes

## Data managing
All data between modules should be handled as structs or array of structs.
E.g. the data for busses when the function is called must return an array of structs built as: {short_name, long_name, ETA, delay} (not final, tbd).
<br>The modules will be divided each into their own file. 