#include "Manchester.h"
#include <tiny_sensor.h>

#define RX_BITRATE MAN_1200

#define RX_PIN 2

packet p_buf, p;

void setup() {
  Serial.begin(9600);
  Serial.println("TinySensor gateway starting up");
  Serial.println(String("Protocol version: ") + TINY_SENSOR_VERSION);
  Serial.println(String("Manchester bitrate: ") + RX_BITRATE);
  man.setupReceive(RX_PIN, RX_BITRATE);
  man.beginReceiveArray(sizeof(packet), (uint8_t *)&p_buf);
}

void loop() {
  if (man.receiveComplete()) {
    memcpy(&p, &p_buf, sizeof(packet));

    // Start listening for next message right after you retrieve the message
    man.beginReceiveArray(sizeof(packet), (uint8_t *)&p_buf);

    Serial.println();
    Serial.println("-- Packet received");
    Serial.println(String("sensor: ver=") + p.version + String(", id=") + p.sensor_id + String(", seq=") + p.seq);

    uint8_t calculated_crc = crc8((uint8_t *)&p, sizeof(packet) - 1);
    if (calculated_crc != p.crc) {
      Serial.println(String("!! CRC is incorrect: calculated=") + String(calculated_crc, HEX) + \
          String(", received=") + String(p.crc, HEX));
    } else {
      Serial.println(String("cap: temp=") + describeTempCap(&p) + \
        String(", sw=") + describeSwCap(&p) + \
        String(", ldr=") + describeLdrCap(&p));
      Serial.println("values:");
      Serial.println(String("  vcc=") + p.state.vcc + String(" mV"));
      if (p.cap.sw != CAP_SW_NONE) {
        Serial.println(String("  sw.val=") + p.state.sw.val);
        Serial.println(String("  sw.intr=") + p.state.sw.intr);
      }
      if (p.cap.temp != CAP_TEMP_NONE) {
        Serial.println(String("  temp=") + String(p.state.temp / 100.0, 2) + " C");
        if (p.cap.temp == CAP_TEMP_DHT11 || p.cap.temp == CAP_TEMP_DHT22) {
          Serial.println(String("  humid=") + String(p.state.humid / 100.0, 2) + " %rH");
        }
      }
      if (p.cap.ldr == CAP_LDR_PRESENT) {
        Serial.println(String("  lux=") + p.state.lux);
      }
    }
  }
}

String describeTempCap(packet *p) {
  switch (p->cap.temp) {
    case CAP_TEMP_NONE:     return "n/a";
    case CAP_TEMP_DHT11:    return "dht11";
    case CAP_TEMP_DHT22:    return "dht22";
    case CAP_TEMP_DS18B20:  return "ds18b20";
    default:                return "unsupported";
  }
}

String describeSwCap(packet *p) {
  switch (p->cap.sw) {
    case CAP_SW_NONE: return "n/a";
    case CAP_SW_REED: return "reed";
    case CAP_SW_PIR:  return "pir";
    default:          return "unsupported";
  }
}

String describeLdrCap(packet *p) {
  switch (p->cap.ldr) {
    case CAP_LDR_NONE: return "n/a";
    case CAP_LDR_PRESENT: return "present";
  }
}
