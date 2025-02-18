/*
Build a more advanced project where you integrate multiple sensors 
(for example, a DHT sensor for temperature/humidity, an ultrasonic 
sensor for distance, and an IR sensor for motion) and have each 
sensor read performed in its own task. Each task updates a shared 
global structure holding the latest sensor values. Finally print 
the read values to the Serial terminal using another task, while 
taking the mutex.

How It Works:
Sensor Tasks:
DHT Task: Reads temperature and humidity.
Ultrasonic Task: Measures distance.
IR Task: Detects motion or presence.
Each task locks the mutex before updating its respective fields in a 
shared structure and then unlocks it.
Serial Monitor Printing Task: Periodically locks the mutex, 
prints the complete sensor data, unlocks the mutex.

*/

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

#include <DHT.h>

struct SensorData {
  float DHT11temperature;  // To store temperature
  float DHT11humidity;
  float Ultrasonic_duratn;
  uint8_t Ultrasonic_len;  // To store length
  bool IR_presence;
};

SensorData data;

static SemaphoreHandle_t mutex;

#define DHT11_PIN 4
#define DHTTYPE DHT11


#define TRIG_PIN 23  // Trigger pin connected to GPIO23
#define ECHO_PIN 22  // Echo pin connected to GPIO22
#define IR_PIN 5     // IR sensor pin (adjust to your pin)

DHT DHT11_Sensor(DHT11_PIN, DHTTYPE);


void DHTTask(void *parameter) {
  while (1) {

    xSemaphoreTake(mutex, portMAX_DELAY);  //take mutex

    data.DHT11humidity = DHT11_Sensor.readHumidity();
    data.DHT11temperature = DHT11_Sensor.readTemperature();

    xSemaphoreGive(mutex);                  //give mutex
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay for 2 seconds before reading again
  }
}


void PrintTask(void *parameter) {
  while (1) {

    xSemaphoreTake(mutex, portMAX_DELAY);  //take mutex

    Serial.print("Humidity: ");
    Serial.print(data.DHT11humidity);
    Serial.println(" % , ");
    Serial.print("Temperature: ");
    Serial.print(data.DHT11temperature);
    Serial.println(" °C , ");

    // Print the result to the Serial Monitor
    Serial.print("Distance: ");
    Serial.print(data.Ultrasonic_len);  // Print distance in centimeters
    Serial.println(" cm");

    Serial.print("IR Presence: ");
    Serial.println(data.IR_presence ? "Not Detected" : "Detected");

    xSemaphoreGive(mutex);  //give mutex

    vTaskDelay(1000 / portTICK_PERIOD_MS);  //for button debouncing
  }
}


void UltrasonicTask(void *parameter) {
  while (1) {
    xSemaphoreTake(mutex, portMAX_DELAY);  //take mutex

    digitalWrite(TRIG_PIN, LOW);   // Ensure the trigger pin is LOW to start
    delayMicroseconds(2);          // Wait for 2 microseconds
    digitalWrite(TRIG_PIN, HIGH);  // Set Trigger pin HIGH to send the pulse
    delayMicroseconds(10);         // Wait for 10 microseconds
    digitalWrite(TRIG_PIN, LOW);   // Set Trigger pin back to LOW

    // Read the duration of the Echo pulse
    data.Ultrasonic_duratn = pulseIn(ECHO_PIN, HIGH);  // Measure the pulse duration

    // Calculate the distance in centimeters (speed of sound: 343 meters per second)
    data.Ultrasonic_len = data.Ultrasonic_duratn * 0.0344 / 2;  // Divide by 2 because pulse travels to the object and back
    xSemaphoreGive(mutex);                                      //give mutex
    vTaskDelay(500 / portTICK_PERIOD_MS);                       // Delay for 0.5 seconds before checking again
  }
}

void IRTask(void *parameter) {
  while (1) {
    xSemaphoreTake(mutex, portMAX_DELAY);  // Take mutex
    data.IR_presence = digitalRead(IR_PIN); //read the presence
    xSemaphoreGive(mutex);                 // Release mutex
    vTaskDelay(500 / portTICK_PERIOD_MS);  // Delay for 0.5 seconds before checking again
  }
}



void setup() {

  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);  // Set Trigger pin as output
  pinMode(ECHO_PIN, INPUT);   // Set Echo pin as input
  pinMode(IR_PIN, INPUT);     // Set IR sensor pin as input

  DHT11_Sensor.begin();

  mutex = xSemaphoreCreateMutex();  //created the semaphore


  xTaskCreatePinnedToCore(DHTTask,"DHT11 Task",2048,NULL,2,NULL,app_cpu);
  xTaskCreatePinnedToCore(PrintTask,"Print the data",1024,NULL,1,NULL,app_cpu);
  xTaskCreatePinnedToCore(UltrasonicTask,"Ultrasonic Task",2048,NULL,2,NULL,app_cpu);
  xTaskCreatePinnedToCore(IRTask,"IR Task",1024,NULL,2,NULL,app_cpu);

  vTaskDelete(NULL);

  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}
