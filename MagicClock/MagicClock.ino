#include <Arduino.h>
#include <cerrno>
#include <cstring>
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

const char location_cmd[] = "/location";
const char step_cmd[] = "/step";

// Change the first parameter to switch the direction of rotation
// TODO: Verify pinouts
ClockHand personA = ClockHand(true, POSITIONS, SPEED, STEPS, MODE, 2, 3, 4, 5);
ClockHand personB = ClockHand(false, POSITIONS, SPEED, STEPS, MODE, 6, 7, 8, 9);
ClockHand personC = ClockHand(true, POSITIONS, SPEED, STEPS, MODE, 10, 11, 12, 13);
ClockHand personD = ClockHand(false, POSITIONS, SPEED, STEPS, MODE, 14, 15, 16, 17);
ClockHand personE = ClockHand(true, POSITIONS, SPEED, STEPS, MODE, 18, 19, 20, 21);
ClockHand *active;

void setup() {
  // Serial console setup
  Serial.begin(9600);

  // Wi-Fi/MQTT setup
  wifi_reconnect();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(msgReceived);

  // Set up EEPROM - necessary because Nano ESP32 immitates EEPROM with a RAM block stored in flash
  EEPROM.begin(s_CLOCK_HAND_COUNT * ClockHand::s_EEPROM_ADDR_STEP);

  // Set up clock hands
  personA.setup();
  personB.setup();
  personC.setup();
  personD.setup();
  personE.setup();

  Serial.println("Setup complete.");
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
// 0: Work
// 1: School
// 2: Relative's
// 3: Traveling
// 4: Lost
// 5: Mortal Peril
// 6: Doctor
// 7: Shopping
// 8: Restaurant
// 9: Home

int parseLocation(char loc[]) {
  int pos = 4; // default to Lost
  if (std::strcmp(loc,"Work") == 0) {
    pos = 0;
  }
  else if (std::strcmp(loc,"School") == 0) {
    pos = 1;
  }
  else if (std::strcmp(loc,"Relative's") == 0) {
    pos = 2;
  }
  else if (std::strcmp(loc,"Traveling") == 0) {
    pos = 3;
  }
  else if (std::strcmp(loc,"Lost") == 0) {
    pos = 4;
  }
  else if (std::strcmp(loc,"Mortal Peril") == 0) {
    pos = 5;
  }
  else if (std::strcmp(loc,"Doctor") == 0) {
    pos = 6;
  }
  else if (std::strcmp(loc,"Shopping") == 0) {
    pos = 7;
  }
  else if (std::strcmp(loc,"Restaurant") == 0) {
    pos = 8;
  }
  else if (std::strcmp(loc,"Home") == 0) {
    pos = 9;
  }
  else {
    Serial.print("  Invalid location: ");
    Serial.println(loc);
    Serial.print("    Defaulting to Lost - ");
    Serial.println(pos);
  }
  return pos;
}

long parseStepCount(char steps[]) {
  long step_count = std::strtol(steps, NULL, 10);
  if (errno == ERANGE) {
    Serial.print("  Invalid step count: ");
    Serial.println(steps);
    Serial.println("    Defaulting to 0 steps.");
    step_count = 0;
  }
  return step_count;
}

void msgReceived(char* topic, uint8_t* payload, unsigned int length) {

  // Log the message to serial
  Serial.println("Received a message.");
  Serial.print("  Topic: ");
  Serial.println(topic);

  // Select the appropriate person based on the topic
  if (strstr(topic, "home/clock/personA") != NULL) {
    active = &personA;
    Serial.print("  Moving Person A ");
  } else if (strstr(topic, "home/clock/personB") != NULL) {
    active = &personB;
    Serial.print("  Moving Person B ");
  } else if (strstr(topic, "home/clock/personC") != NULL) {
    active = &personC;
    Serial.print("  Moving Person C ");
  } else if (strstr(topic, "home/clock/personD") != NULL) {
    active = &personD;
    Serial.print("  Moving Person D ");
  } else if (strstr(topic, "home/clock/personE") != NULL) {
    active = &personE;
    Serial.print("  Moving Person E ");
  } else {
    Serial.println("  ***Invalid topic received***");
    Serial.println("    Format is 'home/clock/<persons_name>/<command>'");
    return;
  }

  // Unpack the payload to a character buffer
  char message[length+1];
  for (int i = 0; i < length; i++) {
    message[i] = payload[i];
  }
  message[length] = '\0';
  Serial.print(" Message is: ");
  Serial.println(message);

  std::size_t topic_len = strlen(topic);
  if (std::strncmp(topic + topic_len - sizeof(location_cmd), location_cmd, sizeof(location_cmd))) {
    int pos = parseLocation(message);
    Serial.println("  Move to position");
    Serial.print("  position index: ");
    Serial.println(pos);
    active->setNewPosition(pos);
  } else if (std::strncmp(topic + topic_len - sizeof(step_cmd), step_cmd, sizeof(step_cmd))) {
    long steps = parseStepCount(message);
    Serial.println("  Step offset");
    Serial.print("  step count: ");
    Serial.println(steps);
    active->move(steps);
  } else {
    Serial.println("  Invalid command - the following are accepted commands:");
    Serial.print("    ");
    Serial.println(location_cmd);
    Serial.print("    ");
    Serial.println(step_cmd);
  }
}