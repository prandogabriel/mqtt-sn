# Environmental Stations

Using your favorite programming language you need to create a stand-alone program that represents a virtual environmental station that generates periodically a set of random values for 5 different sensors:

1. Temperature (-50 ... 50 Celsius)
2. Humidity (0 ... 100%)
3. Wind direction (0 ... 360 degrees)
4. Wind intensity (0 ... 100 m/s)
5. Rain height (0 ... 50 mm / h)

The virtual environmental station uses a unique ID (identity) to publish these random values on an MQTT channel. You need to have at least 2 such virtual stations running and publishing their values on the MQTT channel.

## How To

To generate values and to send them to your AWS - IoT Core Thing with the MQTT protocol:
```
python3 environmental_station_simulator.py --clientid stationID
```
where stationID is supposed to be station1 or station2 in this case. Then, the provided data will be sent to the database (DynamoDB).
NB: you have to change some personal paramteres in the code.
