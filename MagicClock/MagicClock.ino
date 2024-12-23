#include <Arduino.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "ClockHand.h"
#include "config.h"

#define STEPS 4096
#define MODE 8
#define SPEED 500
#define POSITIONS 10

constexpr uint8_t s_CLOCK_HAND_COUNT = 5;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Change the first parameter to switch the direction of rotation
// TODO: Verify pinouts
ClockHand personA = ClockHand(true, POSITIONS, SPEED, STEPS, MODE, 2, 4, 3, 5);
ClockHand personB = ClockHand(false, POSITIONS, SPEED, STEPS, MODE, 6, 8, 7, 9);
ClockHand personC = ClockHand(true, POSITIONS, SPEED, STEPS, MODE, 10, 12, 11, 13);
ClockHand personD = ClockHand(false, POSITIONS, SPEED, STEPS, MODE, 14, 16, 15, 17);
ClockHand personE = ClockHand(true, POSITIONS, SPEED, STEPS, MODE, 18, 19, 20, 21);
ClockHand *active;

void setup() {
  // Wi-Fi/MQTT setup
  wifi_reconnect();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(msgReceived);

  // Set up EEPROM - necessary because Nano ESP32 immitates EEPROM with a RAM block stored in flash
  EEPROM.begin(s_CLOCK_HAND_COUNT * ClockHand::s_EEPROM_ADDR_STEP);
  personA.setup();
  personB.setup();
  personC.setup();
  personD.setup();
  personE.setup();

  // Serial console setup
  Serial.begin(9600);
  Serial.println("Startup complete.");
}

void loop() {
  if(WiFi.status() != WL_CONNECTED) {  
    wifi_reconnect();
  }
  if(!client.connected()) {
    mqtt_reconnect();
  }
  
  client.loop();

  personA.run();
  personB.run();
  personC.run();
  personD.run();
  personE.run();
}

void wifi_reconnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void mqtt_reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(HOSTNAME,MQTT_USER,MQTT_PASS)) {
      Serial.println("connected");
      client.publish("home/clock","Connected");
      client.subscribe("home/clock/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Clock face positions:
// 0: Home
// 1: Work
// 2: School
// 3: Relative's
// 4: Traveling
// 5: Lost
// 6: Mortal Peril
// 7: Doctor
// 8: Shopping
// 9: Restaurant

int parseLocation(char loc[]) {
  int pos=3; // default to Lost
  if (strcasecmp(loc,"Home") == 0) {
    pos = 0;
  }
  else if (strcasecmp(loc,"Work") == 0) {
    pos = 1;
  }
  else if (strcasecmp(loc,"School") == 0) {
    pos = 2;
  }
  else if (strcasecmp(loc,"Relative's") == 0) {
    pos = 3;
  }
  else if (strcasecmp(loc,"Traveling") == 0) {
    pos = 4;
  }
  else if (strcasecmp(loc,"Lost") == 0 or strcasecmp(loc,"not_home") == 0) {
    pos = 5;
  }
  else if (strcasecmp(loc,"Mortal Peril") == 0) {
    pos = 6;
  }
  else if (strcasecmp(loc,"Doctor") == 0) {
    pos = 7;
  }
  else if (strcasecmp(loc,"Shopping") == 0) {
    pos = 8;
  }
  else if (strcasecmp(loc,"Restaurant") == 0) {
    pos = 9;
  }
  else {
    Serial.println("Invlid location.");
  }
  return pos;
}

void msgReceived(char* topic, byte* payload, unsigned int length) {
  // Log the message to serial
  Serial.println("Received a message.");
  Serial.print("  Topic: ");
  Serial.println(topic);

  // Select the appropriate person based on the topic
  if(strcmp(topic,"home/clock/personA") == 0) {
    active = &personA;
    Serial.print("  Moving Person A ");
  }
  else if(strcmp(topic,"home/clock/personB") == 0) {
    active = &personB;
    Serial.print("  Moving Person B ");
  }
  else if(strcmp(topic,"home/clock/personC") == 0) {
    active = &personC;
    Serial.print("  Moving Person C ");
  }
  else if(strcmp(topic,"home/clock/personD") == 0) {
    active = &personD;
    Serial.print("  Moving Person D ");
  }
  else if(strcmp(topic,"home/clock/personE") == 0) {
    active = &personE;
    Serial.print("  Moving Person E ");
  }
  else {
    Serial.println("  ***Invalid topic received.***");
    return;
  }

  // Unpack the payload to a character buffer
  char message[length+1];
  for (int i = 0; i < length; i++) {
    message[i] = payload[i];
  }
  message[length] = '\0';
  Serial.print(" to ");
  Serial.println(message);

  int pos = parseLocation(message);
  Serial.print("  position index: ");
  Serial.println(pos);
  active->setNewPosition(pos);
}