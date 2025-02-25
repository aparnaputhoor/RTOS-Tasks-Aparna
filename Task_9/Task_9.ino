#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

#include <DHT.h>

QueueHandle_t xQueue;

struct SensorData {
  float DHT11temperature;  // To store temperature
  float DHT11humidity;
};

struct SensorData data;

#define DHT11_PIN 18
#define DHTTYPE DHT11

DHT DHT11_Sensor(DHT11_PIN, DHTTYPE);

void readDHT11(void *parameter) {

  while (1) {
    if (digitalRead(4))  
    {
      data.DHT11humidity = DHT11_Sensor.readHumidity();
    data.DHT11temperature = DHT11_Sensor.readTemperature();
      xQueueSend(xQueue, &data, portMAX_DELAY);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);  // Delay for 2 seconds before
  }
}

void printData(void *parameter) {
  while(1) {
    if (xQueueReceive(xQueue, &data, portMAX_DELAY)) {
      Serial.print("Temperature: ");
      Serial.print(data.DHT11temperature);
      Serial.print(" °C, ");
      Serial.print("Humidity: ");
      Serial.print(data.DHT11humidity);
      Serial.println(" %");
    }
  }
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  pinMode(18, INPUT);
  pinMode(4, INPUT);
 DHT11_Sensor.begin();

  xQueue = xQueueCreate(2, sizeof(struct SensorData));

  xTaskCreatePinnedToCore(readDHT11, "read the data", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(printData, "print the data", 2048, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

   
  // if (digitalRead(4) == HIGH) {
  //   digitalWrite(2, LOW);
  // } else {
  //   digitalWrite(2, HIGH);
  // }
  // delay(500);
}
