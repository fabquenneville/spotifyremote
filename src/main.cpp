#include <Arduino.h>
#include <EEPROM.h>
#include <WiFi.h>

const int btnPrev = 27;
const int btnMid = 25;
const int btnNxt = 32;
const int redLed = 22;

const int wifi_timeout = 20000;

#define EEPROM_SIZE 1024
typedef struct config {
  char ssid[32];
  char pass[32];
  char domain[32];
} Config;
Config config;

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
  Serial.print("Connecting to WiFi ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.ssid, config.pass);
  unsigned long startTime = millis();
  while(WiFi.status() != WL_CONNECTED && millis() - startTime < wifi_timeout){
    Serial.print(".");
    delay(100);
  }
  Serial.print("\r\n");

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("WiFi error!");
  } else {
    Serial.print("Connected, IP: ");
    Serial.println(WiFi.localIP());
  }
}

IPAddress get_ghomeIP(){

  // Building the FQDN
  String hostname = "Google-Home." + String(config.domain);
  const char *chostname = hostname.c_str();

  Serial.println("Looking for " + hostname);
  IPAddress ghomeIP;
  int err = WiFi.hostByName(chostname, ghomeIP);
  if(err == 1){
    Serial.print("Speaker found: ");
    Serial.println(ghomeIP);
  } else {
    Serial.println("Speaker not found.");
  }
  return ghomeIP;
}

void setup() {
  Serial.begin(115200);
  
  // Setting input / outputs
  pinMode(btnPrev, INPUT);
  pinMode(btnMid, INPUT);
  pinMode(btnNxt, INPUT);
  pinMode(redLed, OUTPUT);
  digitalWrite(redLed, LOW);


  // read configuration from EEPROM
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, config);

  // Network
  netconnnect();

  // Find speaker
  IPAddress ghomeIP = get_ghomeIP();
}


void loop() {
  // bool previous = false;
  // bool middle = false;
  // bool next = false;
  if (digitalRead(btnPrev)  == 1) {
    Serial.println("Previous");
    indicate(redLed, 1, false);
  } else if (digitalRead(btnMid)  == 1) {
    Serial.println("Middle");
    indicate(redLed, 2, false);
  } else if (digitalRead(btnNxt)  == 1) {
    Serial.println("Next");
    indicate(redLed, 3, false);
  }
  delay(200);
}
