#include <Wire.h>
#include <Adafruit_VL53L1X.h>
#include <Adafruit_MCP2515.h>

// Adafruit Feather RP2040 CAN
#define CS_PIN PIN_CAN_CS
#define CAN_BAUDRATE 250000

Adafruit_VL53L1X vl53;
Adafruit_MCP2515 mcp(CS_PIN);

// Prototype OMRB module identity
const uint32_t MODULE_ID = 0x12345678;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("OMRB Distance Sensor starting...");

  // Initialize I2C
  Wire.begin();

  // Initialize VL53L1X
  if (!vl53.begin(0x29, &Wire)) {
    Serial.print("VL53L1X init failed. Status: ");
    Serial.println(vl53.vl_status);
    while (1) delay(10);
  }

  Serial.println("VL53L1X found");

  if (!vl53.startRanging()) {
    Serial.print("Could not start ranging. Status: ");
    Serial.println(vl53.vl_status);
    while (1) delay(10);
  }

  // 50 ms ranging timing budget
  vl53.setTimingBudget(50);

  // Initialize CAN
  if (!mcp.begin(CAN_BAUDRATE)) {
    Serial.println("MCP2515 CAN init failed");
    while (1) delay(10);
  }

  Serial.println("CAN initialized");
  Serial.println("OMRB Distance Sensor READY");
}

void sendAnnouncement() {
  uint8_t announce[8] = {
    0x01,                   // OMRB protocol v1
    0x01,                   // module type: distance sensor
    0x12, 0x34, 0x56, 0x78, // module ID
    0x01,                   // capability: distance measurement
    0x01                    // state: READY
  };

  mcp.beginPacket(0x100);
  mcp.write(announce, sizeof(announce));
  mcp.endPacket();
}

void sendDistance(uint16_t distanceMm) {
  uint8_t data[6] = {
    0x12,
    0x34,
    0x56,
    0x78,
    (uint8_t)(distanceMm >> 8),
    (uint8_t)(distanceMm & 0xFF)
  };

  mcp.beginPacket(0x101);
  mcp.write(data, sizeof(data));
  mcp.endPacket();
}

void loop() {
  static unsigned long lastAnnouncement = 0;

  // Periodically advertise this module and its capability
  if (millis() - lastAnnouncement >= 5000) {
    sendAnnouncement();
    lastAnnouncement = millis();

    Serial.println("OMRB ANNOUNCE sent");
  }

  // Publish real distance measurements
  if (vl53.dataReady()) {
    int16_t distance = vl53.distance();

    if (distance >= 0) {
      Serial.print("Distance: ");
      Serial.print(distance);
      Serial.println(" mm");

      sendDistance((uint16_t)distance);
    }

    vl53.clearInterrupt();
  }
}