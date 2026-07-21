#!/bin/bash
# Certificate generation script example, modify it to generate certificates
# for more clients if required.

set -e

# ── 1. Certificate Authority ──────────────────────────────────────────
openssl genrsa -out ca.key 2048
openssl req -new -x509 -days 3650 -key ca.key -out ca.crt \
  -subj "/CN=SmartCity-CA"

# ── 2. Mosquitto broker certificate ──────────────────────────────────
openssl genrsa -out server.key 2048
openssl req -new -key server.key -out server.csr \
  -subj "/CN=edgex-mqtt-broker" # CHANGE THIS
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key \
  -CAcreateserial -out server.crt -days 3650

# ── 3. Client certificate (sensor node) ──────────────────────────────
# Replace NODE_ID with a unique identifier per device
# e.g. esp32-node-01, stm32-node-01, etc.
NODE_ID=${1:-"sensor-node-01"}

openssl genrsa -out "${NODE_ID}.key" 2048
openssl req -new -key "${NODE_ID}.key" -out "${NODE_ID}.csr" \
  -subj "/CN=${NODE_ID}"
openssl x509 -req -in "${NODE_ID}.csr" -CA ca.crt -CAkey ca.key \
  -CAcreateserial -out "${NODE_ID}.crt" -days 3650

# ── 4. NiFi PKCS12 keystore and truststore ───────────────────────────
# Notice the 'changeit' placeholder
openssl pkcs12 -export \
  -in "${NODE_ID}.crt" -inkey "${NODE_ID}.key" \
  -out client-keystore.p12 -passout pass:changeit

openssl pkcs12 -export \
  -in ca.crt -nokeys \
  -out truststore.p12 -passout pass:changeit

echo ""
echo "Done. Files generated:"
echo "  CA:         ca.crt, ca.key"
echo "  Broker:     server.crt, server.key"
echo "  Client:     ${NODE_ID}.crt, ${NODE_ID}.key"
echo "  NiFi:       client-keystore.p12, truststore.p12"
echo ""
echo "IMPORTANT: Add ca.crt, server.crt, server.key to your"
echo "mosquitto/config/certs/ directory before starting the broker."
echo "Embed ca.crt, ${NODE_ID}.crt, and ${NODE_ID}.key into"
echo "your sensor node firmware before flashing (Arduino Nano ESP32) "
echo "or via Tera Term after flashing (STM32 Discovery). "