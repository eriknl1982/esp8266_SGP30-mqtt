/***************************************************************************
  Sample sketch for using a sgp30 air quality sensor with a ESP8266 and sending the result over MQTT once a minute. 

  Written by Erik Lemcke, combined out of the following samples:

  https://learn.adafruit.com/adafruit-sgp30-gas-tvoc-eco2-mox-sensor/arduino-code SGP30 code^& library by adafruit 
  https://www.home-assistant.io/blog/2015/10/11/measure-temperature-with-esp8266-and-report-to-mqtt/, home assistant mqqt by Paulus Schoutsen

***************************************************************************/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Wire.h>
#include <SPI.h>

#include "Adafruit_SGP30.h"

Adafruit_SGP30 sgp;

#define wifi_ssid "YOUR WIFI SSID"
#define wifi_password "YOUR WIFI PASSWORD"

#define mqtt_server "YOUR HOME ASSISTANT IP"
#define mqtt_user "YOUR MQTT USER"
#define mqtt_password "YOUR MQTT PASSWORD"

#define co2_topic "sgp30/co2" //change to whatever topic you like

WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.mode(WIFI_STA); //Do not expose as access point
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  
  Serial.begin(115200);
  Serial.println("Startup");

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  
  unsigned long previousMillis = millis();

 if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    while (1);
  }
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
  delay(500);
}

long lastMsg = 0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  //send a meaage every minute
  if (now - lastMsg > 60 * 1000) {
    lastMsg = now;

     if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");
  delay(1000);  

  client.publish(co2_topic, String(sgp.eCO2).c_str(), true);
    
    }
  }
