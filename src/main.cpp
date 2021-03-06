#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <WiFiClient.h>
#include <DHT.h>

#define SECOND                        1000
#define DHTPIN                           4 // DHT sensor data pin see pinout https://pradeepsinghblog.files.wordpress.com/2016/04/nodemcu_pins.png?w=616
#define DHTTYPE                      DHT22 // which DHT model we use 22
#define WAIT_UNTIL_NEW_PACKAGE 10 * SECOND // How long we wait to send a new data package 
#define WUID                 ""
#define WUPASS                  ""

WiFiClient wifiClient;                   // Wireless network
DHT        dht(DHTPIN, DHTTYPE);         // DHT Temperature and Humidity Sensor
uint8_t    connection_error_counter = 0;

void setup() {

  WiFiManager wifiManager;
  //wifiManager.resetSettings();              // for debugging, this resets the configuration and forces the wifimanager to ask for the wifi network
  wifiManager.setConfigPortalTimeout(60 * 5); // 5 minutes timeout in case of a power failure without direct wireless access
    
  if (!wifiManager.autoConnect("WeatherML")) {
    ESP.reset(); // In case there is no answer within the timeout periode reset and try again to connect to the already known network
    delay(1000);
  }

  dht.begin();
}

void pushToWunderground(){

  String url = "/weatherstation/updateweatherstation.php?ID=";
         url += WUID;
         url += "&PASSWORD=";
         url += WUPASS;
         url += "&dateutc=now";
         url += "&tempf=";
         url += dht.convertCtoF(dht.readTemperature());
         url += "&humidity=";
         url += dht.readHumidity();
         url += "&softwaretype=WeatherML&action=updateraw";
  Serial.println(url);

  WiFiClient client;

  if (!client.connect("weatherstation.wunderground.com", 80)) {
    connection_error_counter++;
    return;
  }

  client.print(String("GET ") + url + " HTTP/1.1\r\nHost: weatherstation.wunderground.com\r\nConnection: close\r\n\r\n");
  delay(10);
}

void loop() {

  if (connection_error_counter > 10){
    ESP.reset();
  }

  pushToWunderground();
  delay(WAIT_UNTIL_NEW_PACKAGE);

}
