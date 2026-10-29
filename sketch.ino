#include <WiFi.h>
#include <HTTPClient.h>
#include "soc/gpio_reg.h"

const int SENSOR_PIN = 4;   // Potentiometer SIG (ADC2)
const int LED_PIN    = 2;   // Status LED

const char* SSID = "Wokwi-GUEST";
const char* PASS = "";

unsigned long lastCycle = 0;
bool ledState = false;

// LED control via registers (GPIO 2 < 32)
inline void ledOn()  { *((volatile uint32_t*)GPIO_OUT_W1TS_REG) = (1UL << LED_PIN); }
inline void ledOff() { *((volatile uint32_t*)GPIO_OUT_W1TC_REG) = (1UL << LED_PIN); }

bool connectWiFi(unsigned long timeoutMs = 8000) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS, 6);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > timeoutMs) return false;
    delay(100);
  }
  return true;
}

// ADC2 (GPIO 4) Wi-Fi chalu thakle kaj kore na,
// tai ekdom choto shomoyer jonno Wi-Fi off kore average read kori
int readSensorStable() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(50);

  long sum = 0;
  const int N = 8;
  for (int i = 0; i < N; i++) {
    sum += analogRead(SENSOR_PIN);
    delay(2);
  }
  return (int)(sum / N);   // 0-4095
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);   // direction setup only
  analogReadResolution(12);
  ledOff();

  Serial.print("Connecting to WiFi...");
  if (connectWiFi()) Serial.println(" connected");
  else Serial.println(" failed (will retry)");
}

void loop() {
  if (millis() - lastCycle < 3000) return;
  lastCycle = millis();

  // 1) LED toggle (registers)
  ledState = !ledState;
  if (ledState) ledOn(); else ledOff();

  // 2) Sensor read (Wi-Fi shamoyik off)
  int value = readSensorStable();
  Serial.print("Sensor: ");
  Serial.println(value);

  // 3) Wi-Fi abar on, tarpor HTTP GET (transmit-er shomoy Wi-Fi active)
  if (!connectWiFi()) {
    Serial.println("WiFi reconnect failed");
    return;
  }

  HTTPClient http;
  String url = "http://httpbin.org/get?sensor=" + String(value);
  http.begin(url);
  http.setTimeout(2500);
  int code = http.GET();
  Serial.print("HTTP code: ");
  Serial.println(code);
  http.end();
}
