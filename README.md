# ESP8266.EnvironmentSensor

Sensor for environment data (temperature and humidity at the moment) based on ESP8266 and DHT22/AM2302. Data is transmitted to a MQTT broker by WIFI

- CPU:    ESP8266 (ESP-01S)
- Sensor: AM2302
- IDE:    Arduino

Libraries used

- ESP8266WiFi by Arduino
- PubSubClient by Nick O'Leary, Version 2.8.0
- DHT Sensor Library by Adafruit, Version 1.4.4

TODO:

- Implement sleep / power-down
