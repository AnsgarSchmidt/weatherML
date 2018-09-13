#include <FS.h> 
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <DHT.h>

#define SECOND                        1000
#define DHTPIN                           4 // DHT sensor data pin see pinout https://pradeepsinghblog.files.wordpress.com/2016/04/nodemcu_pins.png?w=616
#define DHTTYPE                      DHT22 // which DHT model we use 22
#define WAIT_UNTIL_NEW_PACKAGE 10 * SECOND // How long we wait to send a new data package 
#define WUID                 "IBERLIN1817"
#define WUPASS                  "xbi8g6f6"

WiFiClient wifiClient;                   // Wireless network
DHT        dht(DHTPIN, DHTTYPE);         // DHT Temperature and Humidity Sensor
uint8_t    connection_error_counter = 0;
char       wuid_d[40];
char       wupasswd_d[40];


void setup() {
  Serial.begin(9600);
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read



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
