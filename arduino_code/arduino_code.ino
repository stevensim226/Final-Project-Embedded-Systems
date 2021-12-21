#include "DHT.h"
#include "TM1637.h"

#define DHTPIN A8 
#define DHTTYPE DHT22
#define MQ2pin A7
#define CLK 2
#define DIO 3
#define BUZZ_PIN 8
#define BTN_INT_PIN 21

unsigned long lastInterrupt = millis();

TM1637 tm1637(CLK,DIO);
DHT dht(DHTPIN, DHTTYPE);

const char* ATMEGA_INIT = "00000000\n";
const char* ATMEGA_NORMAL = "11111111\n";
const char* ATMEGA_FIRE = "22222222\n";

const float TEMP_THRES = 40.0;
const float SMOKE_THRES = 300.0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial.println("Ready");

  while (true) {
    String s = "";
    boolean t = false;
    while(Serial1.available() > 0) {
       s = Serial1.readString();
       t = true;
    }

    if (t){
      Serial.println(s);

      if (s.indexOf("ted") != -1) {
        break;
      }
    }
  }
  Serial1.write("OK");

  tm1637.init();
  tm1637.set(BRIGHT_TYPICAL);
  dht.begin();
  Serial.println("DHT22 and Gas sensor!");
  Serial2.write(ATMEGA_INIT);
  attachInterrupt(digitalPinToInterrupt(BTN_INT_PIN), interruptServiceRoutine, RISING);
  Serial.println("Interrupt attached");
}

int counter = 0;
bool extFireFlag = false;
float temp;
float smoke;

void loop() {
  Serial.println("Reading sensor data...");
  delay(1000);
  while (true) {
    String recv_str = "";
    boolean t = false;
    while(Serial1.available() > 0) {
       recv_str = Serial1.readString();
       t = true;

       if (recv_str.indexOf("FIRE") != -1) {
          extFireFlag = true;
       } else if (recv_str.indexOf("SAFE") != -1) {
          extFireFlag = false;
       }
    }

    if (t){
      Serial.print(recv_str);
      temp = readTemp();
      smoke = readSmoke();
      String sensorData = String(temp) + "/" + String(smoke);
      Serial.println(sensorData);
      displayTemp(temp);
      Serial1.write(sensorData.c_str());

      if (temp > TEMP_THRES || smoke > SMOKE_THRES || extFireFlag) {
        Serial.println("Fire detected! (2)");
        tone(BUZZ_PIN, 1000);
        Serial2.write(ATMEGA_FIRE); 
      } else {
        Serial.println("Normal situation (1)");
        noTone(BUZZ_PIN);
        Serial2.write(ATMEGA_NORMAL);
      }
       
      delay(5000);
    }
  }
}

void writeString(String stringData) { // Used to serially push out a String with Serial.write()
  for (int i = 0; i < stringData.length(); i++)
  {
    Serial1.write(stringData[i]);   // Push each char 1 by 1 on each loop pass
  }
  Serial.println("Finish writing");
}

float readTemp() {
  float temp = dht.readTemperature();
  if (isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  return temp;
}

float readSmoke() {
  float sensorValue = analogRead(MQ2pin);
  return sensorValue;
}

void displayTemp(float temp) {
  String temp_str = String(temp);
  int gap = 0;
  for (int ii = 0; ii < temp_str.length(); ii++) {
    if (ii == 2) {
      gap = 1;
      continue;
    }
    tm1637.display(ii-gap, temp_str[ii] - '0');
  }
}

void interruptServiceRoutine() {
  if (millis() - lastInterrupt <= 500) return;
  Serial.println("======================");
  Serial.println("External Pressed!");
  Serial.println("======================");
  extFireFlag = !extFireFlag;

  if (extFireFlag) {
    tone(BUZZ_PIN, 1000);
  } else {
    noTone(BUZZ_PIN);
  }
  
  lastInterrupt = millis();
}
