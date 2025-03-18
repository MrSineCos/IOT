#include <WiFi.h>
#include <Wire.h>
#include "DHT20.h"
#include <Arduino_MQTT_Client.h>
#include <ThingsBoard.h>

#define SDA_PIN 21
#define SCL_PIN 22

const char* ssid = "Bonjour";
const char* password = "hellosine";

const char* TOKEN = "aInZbDDLhqg9PaTWZUYr";
const char* THINGSBOARD_SERVER = "app.coreiot.io";
const uint16_t THINGSBOARD_PORT = 1883;

WiFiClient espClient;
Arduino_MQTT_Client mqttClient(espClient);
ThingsBoard tb(mqttClient, 1024);


DHT20 dht20;
QueueHandle_t sensorQueue;

struct SensorData {
  float temperature;
  float humidity;
};

void connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    vTaskDelay(pdMS_TO_TICKS(500));
  }
  Serial.println("\nWiFi Connected");
}

bool connectToThingsBoard() {
  if (tb.connected()) return true;

  Serial.println("Connecting to ThingsBoard...");
  if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
    Serial.println("ThingsBoard connect failed");
    return false;
  }
  Serial.println("Connected to ThingsBoard!");
  return true;
}

// Task 1: Gửi dữ liệu lên ThingsBoard
void ThingsBoard_Task(void *pvParameters) {
  while (true) {
    connectToWiFi();
    if (!connectToThingsBoard()) {
      vTaskDelay(pdMS_TO_TICKS(5000));  // Nếu lỗi, chờ 5 giây rồi thử lại
      continue;
    }

    SensorData data;
    if (xQueueReceive(sensorQueue, &data, pdMS_TO_TICKS(1000)) == pdTRUE) { //Chờ tối đa 1000ms nếu hàng đợi trống. Nếu không có dữ liệu, task tiếp tục vòng lặp.
      tb.sendTelemetryData("temperature", data.temperature);
      tb.sendTelemetryData("humidity", data.humidity);
      Serial.printf("Sent -> Temp: %.2f°C, Humi: %.2f%%\n", data.temperature, data.humidity);
    }
    tb.loop();  // Duy trì kết nối MQTT
    vTaskDelay(pdMS_TO_TICKS(1000));  // Chờ 3 giây trước lần gửi tiếp theo
  }
}

// Task 2: Đọc dữ liệu cảm biến
void Sensor_Task(void *pvParameters) {
  while (true) {
    dht20.read();
    float temp = dht20.getTemperature();
    float humi = dht20.getHumidity();

    if (!isnan(temp) && !isnan(humi)) {
      Serial.printf("Read -> Temp: %.2f°C, Humi: %.2f%%\n", temp, humi);
      SensorData data = { temp, humi };
      xQueueSend(sensorQueue, &data, pdMS_TO_TICKS(500));  // Giới hạn thời gian chờ tối đa 500 ms, nếu sau 500 ms mà queue vẫn đầy thì bỏ qua để tránh treo task
    } else {
      Serial.println("Failed to read DHT20");
    }
    vTaskDelay(pdMS_TO_TICKS(3000));  // Đọc mỗi 3 giây
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  dht20.begin();

  sensorQueue = xQueueCreate(5, sizeof(SensorData));

  xTaskCreate(ThingsBoard_Task, "TBTask", 4096, NULL, 2, NULL);
  xTaskCreate(Sensor_Task, "SensorTask", 4096, NULL, 1, NULL);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));  // Không làm gì
}
