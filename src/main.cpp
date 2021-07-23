#include <ArduinoJson.h>
#include <ArduinoSpotify.h>
#include <ArduinoSpotifyCert.h>
#include <EEPROM.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

const int btnPrev = 27;
const int btnMid = 25;
const int btnNxt = 32;
const int redLed = 22;
const int wifi_timeout = 20000;

String playstatus = "play";

WebServer server(80);

const char *webpageTemplate =
    R"(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />
    <style>
      .grid {
        display: grid;
        grid-template-columns:minmax(0, 1fr);
        grid-row-gap:.1em;
      }
      .grid_double {
          grid-column-start:span 2;		
      }
      .grid_span2{
        grid-column: span 2;
      }
    </style>
  </head>
  <body>
    <h1>%s</h1>
    %s
  </body>
</html>
)";

#define EEPROM_SIZE 1024
typedef struct AppConfig {
  char ssid[32];
  char pass[32];
  char hostname[32];
  char domain[32];
  char spotifyid[64];
  char spotifysecret[64];
} AppConfig;
AppConfig config;

// #define SPOTIFY_REFRESH_TOKEN "AAAAAAAAAABBBBBBBBBBBCCCCCCCCCCCDDDDDDDDDDD"

WiFiClientSecure client;
ArduinoSpotify spotify;

void handleNetworkSetupForm()
{
  Serial.println("In handleInitialSetupForm");
  String message = "Number of args received:";
  message += server.args();
  Serial.println(message);
}

void handleNetworkSetup()
{
  char index[2048];
  const char * title = "SpotifyRemote setup";
  const char * section = R"(
  <section>
  <h2>Initial setup!</h2>
  <form action="/" class="grid grid_double" method="post">
    <label for="ssid">Network SSID:</label>
    <input type="text" id="ssid" name="ssid" required>

    <label for="pass">Network password:</label>
    <input type="text" id="pass" name="pass" required>

    <label for="hostname">Hostname for the remote:</label>
    <input type="text" id="hostname" name="hostname" value="SpotifyRemote" required>

    <label for="domain">Local domain:</label>
    <input type="text" id="domain" name="domain" value="local" required>

    <div class="grid_span2"><input type="submit" value="Submit"></div>
  </form> 
  </section>
  )";
  sprintf(index, webpageTemplate, title, section);
  server.send(200, "text/html", index);
}

void handleSpotifySetupForm()
{
  Serial.println("In handleInitialSetupForm");
  String message = "Number of args received:";
  message += server.args();
  Serial.println(message);
}

void handleSpotifySetup()
{
  char index[2048];
  const char * title = "SpotifyRemote setup";
  const char * section = R"(
  <section>
  <h2>Initial setup!</h2>
  <form action="/" class="grid grid_double" method="post">
    <p class="grid_span2">From <a href="https://developer.spotify.com/dashboard/applications/" target="_blank">Spotify application dashboard</a></p>

    <label for="spotifyid">Spotify client ID:</label>
    <input type="text" id="spotifyid" name="spotifyid" required>

    <label for="spotifysecret">Spotify client secret:</label>
    <input type="text" id="spotifysecret" name="spotifysecret" required>

    <div class="grid_span2"><input type="submit" value="Submit"></div>
  </form> 
  </section>
  )";
  sprintf(index, webpageTemplate, title, section);
  server.send(200, "text/html", index);
}


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
    server.on("/", HTTP_POST, handleNetworkSetupForm);
    server.on("/", handleNetworkSetup);
  } else {
    netconnnect();

    if (String(config.spotifyid).length() == 0){
      apmode();
      server.on("/", HTTP_POST, handleSpotifySetupForm);
      server.on("/", handleSpotifySetup);
    } else {
      spotify.lateInit(client, config.spotifyid, config.spotifysecret);
      client.setCACert(spotify_server_cert);

      Serial.println("Refreshing Access Tokens");
      if(!spotify.refreshAccessToken()){
          Serial.println("Failed to get access tokens");
          return;
      }
    }
  }
  
  server.begin();
}


void loop() {
  server.handleClient();
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
