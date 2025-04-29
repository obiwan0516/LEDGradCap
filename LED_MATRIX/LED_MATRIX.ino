#define BLYNK_TEMPLATE_ID "TMPL2E7Ie-qfs"
#define BLYNK_TEMPLATE_NAME "MatrixDisplay"
#define BLYNK_AUTH_TOKEN "IyYgJr9rcBNq0ksF6KCOGdfYo5rtnOVf"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Replace with your WiFi credentials
char ssid[] = "ICE Border Patrol";
char pass[] = "RowdySailors1899";

// Set up Blynk
char auth[] = BLYNK_AUTH_TOKEN;

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  Blynk.run();
}