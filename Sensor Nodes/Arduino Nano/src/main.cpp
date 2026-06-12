#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <SimpleDHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// --- CONFIGURATION ---
const char* ssid = "my_ssid";
const char* password = "my_password";
const char* mqtt_server = "broker_ip";
const char* mqtt_topic = "sensor";

// --- TLS CERTIFICATES ---
// Paste your CA cert, device cert, and device private key in PEM format below.
// Using raw string literals (R"EOF(...)EOF") avoids having to escape line breaks.

const char* ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
<CA_CERTIFICATE>
-----END CERTIFICATE-----
)EOF";

const char* client_cert = R"EOF(
-----BEGIN CERTIFICATE-----
<CLIENT_CERTIFICATE>
-----END CERTIFICATE-----
)EOF";

const char* client_key = R"EOF(
-----BEGIN PRIVATE KEY-----
<CLIENT_PRIVATE_KEY>
-----END PRIVATE KEY-----
)EOF";


// --- DHT SETUP ---
const int pinDHT11 = D2;
SimpleDHT11 dht11;

WiFiClientSecure espClient;
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

// Sync system clock via NTP — required before TLS, so the
// certificate's validity period can be checked correctly.
void sync_time() {
  configTime(0, 0, "pool.ntp.org");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) { // wait until we get a sane epoch time
    delay(500);
    now = time(nullptr);
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  sync_time();

  // Configure TLS: verify the broker's cert against our CA,
  // and present our own client cert + key for mutual TLS.
  espClient.setCACert(ca_cert);
  espClient.setCertificate(client_cert);
  espClient.setPrivateKey(client_key);

  client.setServer(mqtt_server, 8883);
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
        sizeof(payload), "{\"measurement\":\"sensor_data\", \"tags\":{\"ID\":\"ESP32\"}, \"time\": %lu, \"fields\":{\"Temperature\": %.1f,\"Humidity\": %.1f}}",
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