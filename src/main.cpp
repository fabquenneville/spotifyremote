#include <Arduino.h>
#include <EEPROM.h>
#include <WiFi.h>

const int btnPrev = 27;
const int btnMid = 25;
const int btnNxt = 32;
const int redLed = 22;

const int wifi_timeout = 20000;

#define EEPROM_SIZE 1024
typedef struct credentials {
  char ssid[32];
  char pass[32];
} Credentials;
Credentials credentials;

void indicate(int ledPin, int duration = 3, bool solid = true) {
  if (solid) {
      digitalWrite(ledPin, HIGH);
      delay(1000 * duration);
      digitalWrite(ledPin, LOW);
  } else {
    for (int i = 0; i < duration; i++){
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
      delay(1000);
    }
  }
}

void netconnnect(){
  Serial.println("Connecting to WiFi!");
  WiFi.mode(WIFI_STA);
  WiFi.begin(credentials.ssid, credentials.pass);
  unsigned long startTime = millis();
  while(WiFi.status() != WL_CONNECTED && millis() - startTime < wifi_timeout){
    Serial.print(".");
    delay(100);
  }

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("WiFi error!");
  } else {
    Serial.println("Connected!");
    Serial.println(WiFi.localIP());
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(btnPrev, INPUT);
  pinMode(btnMid, INPUT);
  pinMode(btnNxt, INPUT);
  pinMode(redLed, OUTPUT);
  digitalWrite(redLed, LOW);

  EEPROM.begin(EEPROM_SIZE);
  // read from EEPROM into credentials
  EEPROM.get(0, credentials);
  netconnnect();
}


void loop() {
  // bool previous = false;
  // bool middle = false;
  // bool next = false;
  if (digitalRead(btnPrev)  == 1) {
    indicate(redLed, 2, false);
    Serial.println("Previous");
  } else if (digitalRead(btnMid)  == 1) {
    indicate(redLed, 4, false);
    Serial.println("Middle");
  } else if (digitalRead(btnNxt)  == 1) {
    indicate(redLed, 6, false);
    Serial.println("Next");
  }
  delay(200);
}