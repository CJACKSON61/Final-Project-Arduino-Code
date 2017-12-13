#include <ESP8266WiFi.h>
#include <MQTTClient.h>
#include <WifiUDP.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "0.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

const char ssid[] = "BOTT";
const char password[] = "iotpassword";
const int sensorPin = 4;
int sensorVal = 0;

String date;
String t;
const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"} ;

LiquidCrystal_I2C lcd(0x27, 16, 2);

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

  //timeClient.begin();

  lcd.init();
  lcd.begin(16,2);
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("hello");

  lcd.setCursor(0, 1);      
  lcd.print("world");

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

  client.subscribe("/clock");
  // client.unsubscribe("/hello");
}

void loop() {
  client.loop();
  delay(10); 

  getTime();
  
  sensorVal = digitalRead(sensorPin);
  //Serial.println(sensorVal);
  if(sensorVal == 0){
      //Serial.println("off");
      if (millis() - lastMillis > 1000) {
        lastMillis = millis();
        client.publish("/clock", "clock off");
      }
  } else if(sensorVal == 1){
      //Serial.println("on");
      if (millis() - lastMillis > 1000) {
         lastMillis = millis();
         client.publish("/clock", "clock on");
      }
  }

  if (!client.connected()) {
    connect();
  }
  
  // Wait one second before repeating
  delay (1000);
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

void getTime(){
  date = "";  // clear the variables
  t = "";

  timeClient.update();
  unsigned long epochTime =  timeClient.getEpochTime();

  // convert received time stamp to time_t object
  time_t local, utc;
  utc = epochTime;  
  
  // Then convert the UTC UNIX timestamp to local time
  TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -300};  //UTC - 5 hours - change this as needed
  TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -360};   //UTC - 6 hours - change this as needed
  Timezone usWestern(usPDT, usPST);
  local = usWestern.toLocal(utc);

   // now format the Time variables into strings with proper names for month, day etc
    date += days[weekday(local)-1];
    date += ", ";
    date += months[month(local)-1];
    date += " ";
    date += day(local);
    date += ", ";
    date += year(local);

    // format the time to 12-hour format with AM/PM and no seconds
    t += hourFormat12(local);
    t += ":";
    if(minute(local) < 10)  // add a zero if minute is under 10
      t += "0";
    t += minute(local);
    t += " ";
    t += ampm[isPM(local)];

    lcd.setCursor(0,0);
    lcd.print(t);
    lcd.setCursor(0,1);
    lcd.print(date);
}

