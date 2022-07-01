
#include <ESP8266WiFi.h>                // ESP8266 WiFi
#include <PubSubClient.h>               // MQTT 
#include <DHT.h>                        // DHT-style sensors

#include "secrets.h"                    // WiFi Secrets (SSID and Password)

/****************************** Config */

#define MQTT_BROKER   "127.0.0.1"       // Hostname/IP of the broker
#define MQTT_PORT     1883              // Port of the Broker
#define MAX_TOPICLEN  100               // Max. length of topic string
#define MAX_MSGLEN    100               // Max. length of message string
#define MAX_HNLEN     100               // Max. length of the hostname

#define DHT_TYPE      DHT22             // Type of the DHT sensor we use
#define DHT_PIN       2                 // Number of the DHT data pin

#define LOOPTIME      5000              // Loop in millisecs

#define WITH_SERIAL                     // Enable output on serial console

/****************************** Globals */

DHT           dht(DHT_PIN, DHT_TYPE);   // The DHT sensor object
WiFiClient    espClient;                // The WiFi object
PubSubClient  client(espClient);        // The MQTT object on WiFi
 
char          cBaseTopic[100] = "";     // Systems base topic

/****************************** The Functions */

/**
 * Reconnect to the broker if not connected. Not blocking
 */
void mqtt_reconnect() {
  if (!client.connected()) {
    if (client.connect("ESP8266")) {
      char cTopic[MAX_TOPICLEN];
      snprintf(&cTopic[0], MAX_TOPICLEN, "%s/status", cBaseTopic);
      client.publish(cTopic,"connected");
    }
  }
} // mqtt_reconnect

/**
 * System setup
 */
void setup() {
#ifdef WITH_SERIAL
  // Serial console
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println();
#endif

  // WiFi (Client only)
  WiFi.mode(WIFI_STA);

  // Generate Hostname "ESPSENS_" plus Chip-ID
  char cBuffer[MAX_HNLEN];
  snprintf(&cBuffer[0], MAX_HNLEN, "ESPSENS_%X", system_get_chip_id() );
  WiFi.hostname(cBuffer);
#ifdef WITH_SERIAL
  Serial.printf("Connecting to Wifi as %s ", WiFi.hostname().c_str());
#endif
  WiFi.begin(ssid, password);

  // Wait for WiFi connection
  // TODO: Add timeout with system reset
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
#ifdef WITH_SERIAL
    Serial.print(".");
#endif
  }
#ifdef WITH_SERIAL
  Serial.println("");
  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());  
#endif
  
  // MQTT. Basetopic is "ESPSENS_" plus Chip-ID
  snprintf(&cBaseTopic[0], MAX_TOPICLEN, "ESPSENS_%X", system_get_chip_id() );
  client.setServer(MQTT_BROKER, MQTT_PORT);

  // DHT sensor
  dht.begin();
    
} // setup

/**
 * The main loop
 */
void loop() {
  static unsigned long nextloop = 0;

  // Wait for next loop
  while ( millis() < nextloop) delay(1);
  nextloop = millis() + LOOPTIME;

  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      mqtt_reconnect();
    } else {
      char cTopic[MAX_TOPICLEN];
      char cMessage[MAX_MSGLEN];

      client.loop();

      // Uptime
      snprintf(&cTopic[0], MAX_TOPICLEN, "%s/uptime", cBaseTopic);
      snprintf(&cMessage[0], MAX_MSGLEN, "%lu", millis()/1000);
      client.publish(cTopic, cMessage);

      // Temperature
      float temp = dht.readTemperature();
      if (!isnan(temp)) {
        snprintf(&cTopic[0], MAX_TOPICLEN, "%s/temperature", cBaseTopic);
        snprintf(&cMessage[0], MAX_MSGLEN, "%.1f", temp);
        client.publish(cTopic, cMessage); 
      }

      // Humidity
      float hum = dht.readHumidity();
      if (!isnan(hum)) {
        snprintf(&cTopic[0], MAX_TOPICLEN, "%s/humidity", cBaseTopic);
        snprintf(&cMessage[0], MAX_MSGLEN, "%.1f", hum);
        client.publish(cTopic, cMessage); 
      }

#ifdef WITH_SERIAL
      Serial.print(millis());
      Serial.print(" Temperatur: ");
      Serial.print(temp);
      Serial.print("°C, Luftfeuchtigkeit: ");
      Serial.print(hum);
      Serial.println("%");
#endif
    }
  } else {
    // TODO: If not connected, retry and/or restart system
  }

} // loop
