#include <WiFi.h>
#include <PubSubClient.h>
#include <SimpleDHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// --- CONFIGURATION ---
const char* ssid = "my_ssid";
const char* password = "my_password";
const char* mqtt_server = "broker_ip";
const char* mqtt_topic = "sensor";

// --- DHT SETUP ---
const int pinDHT11 = D2;
SimpleDHT11 dht11;

WiFiClient espClient;
PubSubClient client(espClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

#define LED_PIN D3 

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client")) {
      // Optional: client.subscribe("some/topic");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // NTP setup
  timeClient.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  static unsigned long lastSend = 0;
  if (millis() - lastSend > 15000) { // every 15 seconds

    float temperature = 0.0;
    float humidity = 0.0;
    byte data[64] = {0};
    dht11.read2(pinDHT11, &temperature, &humidity, data);

    timeClient.update();
    unsigned long unixTime = timeClient.getEpochTime();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      char payload[124];

      snprintf(payload,
        sizeof(payload), "{\"measurement\":\"sensor_data\", \"tags\":{\"ID\":\"esp8266\"}, \"time\": %lu, \"fields\":{\"Temperature\": %.1f,\"Humidity\": %.1f}}",
        unixTime,
        temperature,
        humidity);

      client.publish(mqtt_topic, payload);
      Serial.println(payload);
    }

    // Blink LED once to indicate data sent
    digitalWrite(LED_PIN, LOW);   // LED on
    delay(100);                   // short blink
    digitalWrite(LED_PIN, HIGH);  // LED off

    lastSend = millis();
  }
}