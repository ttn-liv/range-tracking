# Range Tracking

Using the Sodaq ONE board with on-board LoRaWAN modem, GPS, low power accelerometer
(with interrupts) to collect signal strength information on TTN gateways.

GPS location is checked on the Sodaq ONE board at then is attempted to be
transmitted via LoRaWAN to a receiving TTN application. Rather than collect the
data on board (which would mean device access to get the logs), the incoming
data is collected as JSON from a node-red server (eg. a Raspberry Pi) and
stored in a SQLite database.

Recovered data from the database can then be converted to GEOJSON lines and
plotted on a graph. 
