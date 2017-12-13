#include "SPI.h"
#include "TFT_22_ILI9225.h"

#include <ESP8266WiFi.h>
#include <MQTTClient.h>

#define TFT_RST LOW //Reset by vin
#define TFT_RS  4   // D1
#define TFT_CS  5  // SS D2
#define TFT_SDI 13  // MOSI D7
#define TFT_CLK 14  // SCK D5
#define TFT_LED 3   // 0 if wired to +5V directly 3.3v

#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)

// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);

const char ssid[] = "BOTT";
const char password[] = "iotpassword";
const int sensorPin = 12; //D6
int sensorVal = 0;

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect();

void setup() {
  
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);
  delay(10);

  // Connect to WiFi network
  WiFi.begin(ssid, password);

  client.begin("10.10.47.203", net);
  client.onMessage(messageReceived);

  connect();
  
  tft.begin();
}

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("arduino", "try", "try")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("/tft");
  // client.unsubscribe("/hello");
}

void loop() {
  client.loop();
  delay(10);
  
  tft.drawRectangle(0, 0, tft.maxX() - 1, tft.maxY() - 1, COLOR_YELLOW);
  tft.fillRectangle(0,0, tft.maxX()-1, tft.maxY()-1, COLOR_YELLOW);
  //tft.setFont(Terminal12x16);
  //tft.drawText(10, 10, "ON!", COLOR_BLACK);
  //tft.setBackgroundColor(COLOR_BLACK);
  
  sensorVal = digitalRead(sensorPin);
  //Serial.println(sensorVal);
  if(sensorVal == 0){
      //Serial.println("off");
      if (millis() - lastMillis > 1000) {
        lastMillis = millis();
        client.publish("/tft", "tft off");
      }
  } else if(sensorVal == 1){
      //Serial.println("on");
      if (millis() - lastMillis > 1000) {
         lastMillis = millis();
         client.publish("/tft", "tft on");
      }
  }
  
  if (!client.connected()) {
    connect();
  }  

  delay(1000);
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

