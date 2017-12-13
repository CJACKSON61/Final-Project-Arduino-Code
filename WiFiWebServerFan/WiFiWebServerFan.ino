#include <ESP8266WiFi.h>
#include <MQTTClient.h>

const char ssid[] = "BOTT";
const char password[] = "iotpassword";

const int sensorPin = 4;
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

  //pinMode(motorPin, OUTPUT);
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

  client.subscribe("/fan");
  // client.unsubscribe("/hello");
}

void loop() {
  client.loop();
  delay(10); 

  sensorVal = digitalRead(sensorPin);
  //Serial.println(sensorVal);
  if(sensorVal == 0){
      //Serial.println("off");
      if (millis() - lastMillis > 1000) {
        lastMillis = millis();
        client.publish("/fan", "fan off");
      }
  } else if(sensorVal == 1){
      //Serial.println("on");
      if (millis() - lastMillis > 1000) {
         lastMillis = millis();
         client.publish("/fan", "fan on");
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

