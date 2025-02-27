/*
Multiple sensor tasks read different sensor values and send their data to a logging task. 
The logging task aggregates these readings and writes them to a shared SD card (or flash) in a log file.

How It Uses Synchronization:
Mutex: Protects the shared file system interface (or shared log buffer)
so only one task writes at a time.

Queue: Each sensor task sends a data structure (timestamp plus sensor value) to a central queue.

Semaphore (Optional): A binary semaphore can signal the logging task when new data 
arrives (or when the queue reaches a certain size) so that it can write immediately rather than polling.

Breadboard/Hardware Setup:
ESP32 board
SD card module
Various sensors (like temperature, light, humidity)

*/

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

#include <DHT.h>
#include <WiFi.h>
#include <time.h>

const char* ssid = "Phoenix@2.4GHz";
const char* password = "Wearenowautonomous";

struct SensorData {
  struct tm timeinfo;
  float sensor_data;  // To store temperature
  uint8_t num;
};

struct SensorData data;

static SemaphoreHandle_t mutex;
static QueueHandle_t queue;

#define DHT11_PIN 4
#define DHTTYPE DHT11


#define TRIG_PIN 23  // Trigger pin connected to GPIO23
#define ECHO_PIN 22  // Echo pin connected to GPIO22
//#define IR_PIN 5     // IR sensor pin 

DHT DHT11_Sensor(DHT11_PIN, DHTTYPE);


void DHTTask(void *parameter) {
  while (1) {
    data.sensor_data = DHT11_Sensor.readTemperature();
     getLocalTime(&data.timeinfo);
      data.num = 2;  // Set flag to indicate temperature data
    xQueueSend(queue, &data, portMAX_DELAY); //send the data in queue
  vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay for 1 seconds before reading again
  }
}

void UltrasonicTask(void *parameter) {
  while (1) {
    digitalWrite(TRIG_PIN, LOW);   // Ensure the trigger pin is LOW to start
    delayMicroseconds(2);          // Wait for 2 microseconds
    digitalWrite(TRIG_PIN, HIGH);  // Set Trigger pin HIGH to send the pulse
    delayMicroseconds(10);         // Wait for 10 microseconds
    digitalWrite(TRIG_PIN, LOW);   // Set Trigger pin back to LOW

    // Read the duration of the Echo pulse
    float Ultrasonic_duratn = pulseIn(ECHO_PIN, HIGH);  // Measure the pulse duration
    // Serial.print("Pulse Duration: ");
    // Serial.println(Ultrasonic_duratn);
    // Calculate the distance in centimeters (speed of sound: 343 meters per second)
    data.sensor_data = (Ultrasonic_duratn * 0.0344 )/ 2;  // Divide by 2 because pulse travels to the object and back
    getLocalTime(&data.timeinfo);
    data.num = 1;
    xQueueSend(queue, &data, portMAX_DELAY);     //send data in queue
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay for 1 second before checking again
  }
}

void PrintTask(void *parameter) {
  while (1) {

    if (xSemaphoreTake(mutex, portMAX_DELAY)==pdTRUE)  //take mutex
    {
      if (xQueueReceive(queue, &data, portMAX_DELAY)) {   //receive the queue
      if (data.num == 2) {                                 //if temperature data comes print it with timestamp
        Serial.print("Temperature: ");
        Serial.print(data.sensor_data);
        Serial.print(" °C, Time: ");
        Serial.print(data.timeinfo.tm_hour);
        Serial.print(":");
        Serial.print(data.timeinfo.tm_min);
        Serial.print(":");
        Serial.println(data.timeinfo.tm_sec);
      } else if (data.num == 1) {                        //if ultrasonic data comes print it with timestamp
       Serial.print("Distance: ");
        Serial.print(data.sensor_data);
        Serial.print(" cm, Time: ");
        Serial.print(data.timeinfo.tm_hour);
        Serial.print(":");
        Serial.print(data.timeinfo.tm_min);
        Serial.print(":");
        Serial.println(data.timeinfo.tm_sec);
      }
      }
      xSemaphoreGive(mutex);                     //give mutex
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);  
  }
}





void setup() {

  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");

  // Set time using NTP
  configTime(0, 0, "pool.ntp.org"); // Connect to an NTP server
  
  pinMode(TRIG_PIN, OUTPUT);  // Set Trigger pin as output
  pinMode(ECHO_PIN, INPUT);   // Set Echo pin as input

  DHT11_Sensor.begin();

  mutex = xSemaphoreCreateMutex();  //created the semaphore
  queue = xQueueCreate(5, sizeof(SensorData));   //created queue with 5 elements

  xTaskCreatePinnedToCore(DHTTask, "DHT11 Task", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(UltrasonicTask, "Ultrasonic Task", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(PrintTask, "Print the data", 1024, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);

  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}
