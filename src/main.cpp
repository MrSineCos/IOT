#include <WiFi.h>
#include <ThingsBoard.h>
#include <ArduinoJson.h>
#include <Arduino_MQTT_Client.h>
#include "DHT20.h"
// WiFi credentials
constexpr char WIFI_SSID[] = "Bonjour";
constexpr char WIFI_PASSWORD[] = "hellosine";

// ThingsBoard credentials
constexpr char TOKEN[] = "rsoMvGsRuM9iNbDZBZd3";
constexpr char THINGSBOARD_SERVER[] = "app.coreiot.io";
constexpr uint16_t THINGSBOARD_PORT = 1883U;

// Baud rate for the debugging serial connection
constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;
constexpr uint16_t MAX_MESSAGE_SEND_SIZE = 256U;
constexpr uint16_t MAX_MESSAGE_RECEIVE_SIZE = 256U;

// telemetry settings
constexpr int16_t TELEMETRY_SEND_INTERVAL = 5000U;
uint32_t previousTelemetrySend;

constexpr char TEMPERATURE_KEY[] = "temperature";
constexpr char HUMIDITY_KEY[] = "humidity";

// Initialize ThingsBoard client
WiFiClient espClient;
Arduino_MQTT_Client mqttClient(espClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_RECEIVE_SIZE, MAX_MESSAGE_SEND_SIZE, Default_Max_Stack_Size);
DHT20 dht20;

void InitWiFi() {
  Serial.println("Connecting to AP ...");
  // Attempting to establish a connection to the given WiFi network
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    // Delay 500ms until a connection has been successfully established
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

const bool reconnect() {
  // Check to ensure we aren't connected yet
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    return true;
  }
  // If we aren't establish a new connection to the given WiFi network
  InitWiFi();
  return true;
}

void setup() {
  // Initialize serial connection for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);
  Serial.println("Hello World");
  Wire.begin();
  dht20.begin();
  // Connect to WiFi
  InitWiFi();
}

void loop() {
  delay(10);

  if (!reconnect()) {
    return;
  }

  // Ensure the connection to ThingsBoard is alive
  if (!tb.connected()) {
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
      Serial.println("Failed to reconnect to ThingsBoard");
      delay(200);
      return;
    }
  }
  if (millis() - previousTelemetrySend > TELEMETRY_SEND_INTERVAL) {

    // Use random value as virtual sensor
    // float temperature = random(20, 40);
    // float humidity = random(50, 100);
    
    // Uncomment if using DHT20
    
    dht20.read();    
    float temperature = dht20.getTemperature();
    float humidity = dht20.getHumidity();
    

    // Uncomment if using DHT11/22
    /*    
    float temperature = 0;
    float humidity = 0;
    dht.read2(&temperature, &humidity, NULL);
    */

    Serial.println("Sending telemetry. Temperature: " + String(temperature, 1) + " humidity: " + String(humidity, 1));

    tb.sendTelemetryData(TEMPERATURE_KEY, temperature);
    tb.sendTelemetryData(HUMIDITY_KEY, humidity);
    tb.sendAttributeData("rssi", WiFi.RSSI()); // also update wifi signal strength
    previousTelemetrySend = millis();
  }

  tb.loop();
//   // Prepare data to send
//   StaticJsonDocument<200> data;
//   data["temperature"] = 25.0;
//   data["humidity"] = 60.0;

//  char payload[256];
//   serializeJson(data, payload);

//   // Send data to ThingsBoard
//   if (tb.sendTelemetryJson(data, sizeof(payload))) {
//     Serial.println("Data sent successfully");
//   } else {
//     Serial.println("Failed to send data");
//   }

//   // Wait for 10 seconds before sending the next data
//   delay(10000);  // Serialize JSON data
 
}

// #include <Arduino.h>
// #include <DHT20.h>

// DHT20 dht20;

// void TaskTemperature_Humidity(void *pvParameters){
//   // Wire.begin(GPIO_NUM_11, GPIO_NUM_12);
//   // dht20.begin();
//   while(1){
//     dht20.read();
//     double temperature = dht20.getTemperature();
//     double humidity = dht20.getHumidity();

//     Serial.print("Temp: "); Serial.print(temperature); Serial.print(" *C ");
//     Serial.print(" Humidity: "); Serial.print(humidity); Serial.print(" %");
//     Serial.println();
    
//     vTaskDelay(5000);
//   }

// }
// void setup() {
//   // put your setup code here, to run once:
//   Serial.begin(115200);
//   Wire.begin();
//   dht20.begin();
//   xTaskCreate(TaskTemperature_Humidity, "LED Control", 2048, NULL, 2, NULL);
// }

// void loop() {
//   // dht20.read();
//   // double temperature = dht20.getTemperature();
//   // double humidity = dht20.getHumidity();

//   // Serial.print("Temp: "); Serial.print(temperature); Serial.print(" *C ");
//   // Serial.print(" Humidity: "); Serial.print(humidity); Serial.print(" %");
//   // Serial.println();
//   // Serial.println("helloworld");
//   // delay(5000);
// }