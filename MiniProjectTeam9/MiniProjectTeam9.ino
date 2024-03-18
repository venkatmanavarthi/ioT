#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include "heltec.h"

// Wifi Credentials
const char* ssid = "wifi_6";
const char* password = "#BossRaLuchas03";

// Server endpoint
const char* serverName = "http://192.168.86.246:60000/post-data";

// DHT Sensor setup
#define DHTPIN 4       // Pin connected to the DHT sensor
#define DHTTYPE DHT22  // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);
float currentTempC = 0.0;
float currentTempF = 0.0;
float currentHumidity = 0.0;

void displayReadingsOnOled() {
  String FtemperatureDisplay = "Temperature: " + (String)currentTempF + "°F";
  String CtemperatureDisplay = "Temperature: " + (String)currentTempC + "°C";
  String humidityDisplay = "Humidity: " + (String)currentHumidity + "%";
  // Clear the OLED screen
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, FtemperatureDisplay);
  Heltec.display->drawString(0, 12, CtemperatureDisplay);
  Heltec.display->drawString(0, 24, humidityDisplay);
  Heltec.display->display();
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!!");
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, false /*Serial Enable*/);
  currentTempC = dht.readTemperature();
  currentTempF = dht.readTemperature(true);
  currentHumidity = dht.readHumidity();
  displayReadingsOnOled();
}

void loop() {
  // Check Wi-Fi connection
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    float humi = dht.readHumidity();
    float tempC = dht.readTemperature();
    float tempF = dht.readTemperature(true);

    if (isnan(tempC) || isnan(tempF) || isnan(humi)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      http.begin(serverName);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      currentTempC = tempC;
      currentTempF = tempF;
      currentHumidity = humi;
      // Prepare data string
      String httpRequestData = "team_number=2";
      httpRequestData += "&temperature=" + String(tempC);
      httpRequestData += "&humidity=" + String(humi);
      httpRequestData += "&timestamp=" + String(millis());

      Serial.println(httpRequestData);
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("success");
        Serial.println(httpResponseCode);
        Serial.println(response);
      } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
      }
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
  delay(10000);  // Send data every 10 seconds
}
