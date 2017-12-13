#include <ESP8266WiFi.h>
#include <MQTTClient.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN   2

#define NUMPIXELS  16



const char ssid[] = "BOTT";
const char password[] = "iotpassword";
const int sensorPin = 4;
int sensorVal = 0;

int high = 150;
int low = 0;

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void connect();

void setup() {
   
  Serial.begin(115200);
  pinMode(sensorPin, INPUT_PULLUP);
  delay(10);
  
  // Connect to WiFi network
  WiFi.begin(ssid, password);

  client.begin("10.10.47.203", net);
  client.onMessage(messageReceived);
  
  connect();
  
  pixels.begin();
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

  client.subscribe("/mytopic");
  // client.unsubscribe("/hello");
}

void loop() {
  client.loop();
  delay(10); 
  
  int color = sin(millis()*0.001)*100;

  //Serial.println(color);

  color = map(color, -100, 100, low, high);

  //Serial.println(color);
  
  for(int i=0;i<NUMPIXELS;i++){

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(color,0,0)); // Moderately bright red color.

    pixels.show(); // This sends the updated pixel color to the hardware.

     // Delay for a period of time (in milliseconds).
  }

  sensorVal = digitalRead(sensorPin);
  //Serial.println(sensorVal);
  if(sensorVal == 0){
      //Serial.println("off");
      if (millis() - lastMillis > 1000) {
        lastMillis = millis();
        client.publish("/mytopic", "LED off");
      }
  } else if(sensorVal == 1){
      //Serial.println("on");
      if (millis() - lastMillis > 1000) {
         lastMillis = millis();
         client.publish("/mytopic", "LED on");
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

