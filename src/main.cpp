#include <Arduino.h>
int btnPrev = 13;
int btnMid = 14;
int btnNxt = 15;

void setup() {
  // put your setup code here, to run once:
  pinMode(btnPrev, INPUT);
  pinMode(btnMid, INPUT);
  pinMode(btnNxt, INPUT);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  // bool previous = false;
  // bool middle = false;
  // bool next = false;
  if (digitalRead(btnPrev)  == 1) Serial.println("Previous");
  else if (digitalRead(btnMid)  == 1) Serial.println("Middle");
  else if (digitalRead(btnNxt)  == 1) Serial.println("Next");
  delay(200);
}