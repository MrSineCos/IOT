#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#ifdef ESP32
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif // ESP32
#endif // ESP8266

void setup()
{
  Serial.begin(115200);
  Serial.println("Hello World");
}

void loop()
{
  // put your main code here, to run repeatedly:
  Serial.println("Looping..");
  delay(1000);
}