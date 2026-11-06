#include <WiFi.h>
#include <HTTPClient.h>
#include "soc/soc.h"
#include "soc/gpio_reg.h"

const int SENSOR_PIN = 4;
const int LED_PIN    = 2;

const char* SSID = "Wokwi-GUEST";
const char* PASS = "";

int sensorValue = 0;
bool ledState = false;

void ledOn()  { REG_WRITE(GPIO_OUT_W1TS_REG, (1UL << LED_PIN)); }
void ledOff() { REG_WRITE(GPIO_OUT_W1TC_REG, (1UL << LED_PIN)); }

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS, 6);
  unsigned long t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < 8000) {
    delay(100);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  analogReadResolution(12);
  ledOff();
}

void loop() {
  unsigned long start = millis();

  // 1) Wi-Fi বন্ধ করে sensor পড়া
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(50);
  sensorValue = analogRead(SENSOR_PIN);
  Serial.print("Sensor: ");
  Serial.println(sensorValue);

  // 2) LED toggle (register দিয়ে)
  ledState = !ledState;
  if (ledState) ledOn(); else ledOff();

  // 3) Wi-Fi চালু করে HTTP GET
  connectWiFi();
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://httpbin.org/get?sensor=" + String(sensorValue));
    http.setTimeout(2500);
    int code = http.GET();
    Serial.print("HTTP code: ");
    Serial.println(code);
    http.end();
  } else {
    Serial.println("WiFi failed");
  }

  // 4) ~3 সেকেন্ডের cycle
  while (millis() - start < 3000) delay(10);
}

