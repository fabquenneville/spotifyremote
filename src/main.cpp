#include <EEPROM.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <ArduinoSpotify.h>
#include <ArduinoSpotifyCert.h>

const int btnPrev = 27;
const int btnMid = 25;
const int btnNxt = 32;
const int redLed = 22;
const int wifi_timeout = 20000;

String playstatus = "play";

#define EEPROM_SIZE 1024
typedef struct config {
  char ssid[32];
  char pass[32];
  char hostname[32];
  char domain[32];
  char spotifyid[64];
  char spotifysecret[64];
} Config;
Config config;

// #define SPOTIFY_REFRESH_TOKEN "AAAAAAAAAABBBBBBBBBBBCCCCCCCCCCCDDDDDDDDDDD"

WiFiClientSecure client;
ArduinoSpotify spotify;


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

void apmode(){
  Serial.println("Setting Access Point         : SpotifyRemote");
  WiFi.softAP("SpotifyRemote");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address                : ");
  Serial.println(IP);

  if (MDNS.begin("spotifyremote"))
  {
    Serial.println("MDNS responder started       : spotifyremote");
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

  if (String(config.ssid).length() == 0){
    apmode();
  } else {
    netconnnect();

    // Network

    spotify.lateInit(client, config.spotifyid, config.spotifysecret);
    client.setCACert(spotify_server_cert);

    Serial.println("Refreshing Access Tokens");
    if(!spotify.refreshAccessToken()){
        Serial.println("Failed to get access tokens");
        return;
    }
  }
  
}


void loop() {
  // bool previous = false;
  // bool middle = false;
  // bool next = false;
  if (digitalRead(btnPrev)  == 1) {
    Serial.print("Going to previous track...");
    if(spotify.previousTrack()){
      Serial.println("done!");
      indicate(redLed, 1, false);
    }
  } else if (digitalRead(btnMid)  == 1) {
    if (playstatus == "play"){
      Serial.print("Pausing...");
      if(spotify.pause()){
        Serial.println("done!");
        playstatus = "pause";
      }
      indicate(redLed, 2, false);
    } else if (playstatus == "pause"){
      Serial.print("Playing...");
      if(spotify.play()){
        Serial.println("done!");
        playstatus = "play";
      }
      indicate(redLed, 2, false);
    }
  } else if (digitalRead(btnNxt)  == 1) {
    Serial.print("Skipping to next track...");
    if(spotify.nextTrack()){
      Serial.println("done!");
      indicate(redLed, 3, false);
    }
  }
  delay(200);
}
