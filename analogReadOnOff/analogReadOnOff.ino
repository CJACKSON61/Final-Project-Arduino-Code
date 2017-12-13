const int sensorPin = 2;
int sensorVal = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(sensorPin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  sensorVal = digitalRead(sensorPin);
  if(sensorVal == 1){
      Serial.println("off");
  } else if(sensorVal == 0){
      Serial.println("on");
    }
}
