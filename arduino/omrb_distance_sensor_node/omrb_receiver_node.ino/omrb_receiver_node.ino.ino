#include <Adafruit_MCP2515.h>

// Adafruit Feather RP2040 CAN
#define CS_PIN PIN_CAN_CS
#define CAN_BAUDRATE 250000

Adafruit_MCP2515 mcp(CS_PIN);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("OMRB Receiver starting...");

  if (!mcp.begin(CAN_BAUDRATE)) {
    Serial.println("MCP2515 init failed");
    while (1) delay(10);
  }

  Serial.println("CAN initialized");
  Serial.println("OMRB Receiver READY");
}

void loop() {
  int packetSize = mcp.parsePacket();

  if (!packetSize) return;

  uint32_t packetId = mcp.packetId();

  // 0x100 = OMRB module announcement
  if (packetId == 0x100 && packetSize == 8) {
    uint8_t data[8];

    for (int i = 0; i < 8; i++) {
      data[i] = mcp.read();
    }

    uint32_t moduleId =
      ((uint32_t)data[2] << 24) |
      ((uint32_t)data[3] << 16) |
      ((uint32_t)data[4] << 8) |
      ((uint32_t)data[5]);

    Serial.println();
    Serial.println("=== OMRB MODULE DISCOVERED ===");

    Serial.print("Protocol: ");
    Serial.println(data[0]);

    Serial.print("Module Type: ");
    Serial.println(
      data[1] == 0x01
        ? "Distance Sensor"
        : "Unknown"
    );

    Serial.print("Module ID: 0x");
    Serial.println(moduleId, HEX);

    Serial.print("Capability: ");
    Serial.println(
      data[6] == 0x01
        ? "Distance Measurement"
        : "Unknown"
    );

    Serial.print("State: ");
    Serial.println(
      data[7] == 0x01
        ? "READY"
        : "Unknown"
    );

    Serial.println("==============================");
  }

  // 0x101 = OMRB distance measurement
  else if (packetId == 0x101 && packetSize == 6) {
    uint8_t data[6];

    for (int i = 0; i < 6; i++) {
      data[i] = mcp.read();
    }

    uint32_t moduleId =
      ((uint32_t)data[0] << 24) |
      ((uint32_t)data[1] << 16) |
      ((uint32_t)data[2] << 8) |
      ((uint32_t)data[3]);

    uint16_t distanceMm =
      ((uint16_t)data[4] << 8) |
      ((uint16_t)data[5]);

    Serial.print("Distance from 0x");
    Serial.print(moduleId, HEX);
    Serial.print(": ");
    Serial.print(distanceMm);
    Serial.println(" mm");
  }
}