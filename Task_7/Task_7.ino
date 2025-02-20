/*
Ambient Light Control with an LDR Sensor
Overview:
Make one task that reads an LDR (light-dependent resistor) sensor to determine 
ambient light and computes a brightness level. A second task then uses PWM to 
adjust an LED's brightness accordingly. Both tasks share a global brightness variable protected by a mutex.

How It Works:
Sensor Task: Periodically reads the analog voltage from the LDR, computes a 
brightness value, and stores it in a shared variable (mutex-protected).
LED Control Task: Reads the brightness value (again within a mutex block) 
and adjusts the LED's PWM duty cycle to dim or brighten the LED.

*/



#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const int led_pin = 2;
static const int ldr_pin = 34;

static SemaphoreHandle_t countingSemaphore;
static int ldr_state = 0;



void ProducerTask(void *parameter) {
  while (1) {  //take mutex
    if (xSemaphoreTake(countingSemaphore, 0))
      ;
    else {
      ldr_state = analogRead(ldr_pin) >> 4;  //analog value from ldr sensor
      xSemaphoreGive(countingSemaphore);     //give mutex
      Serial.println("Producer give the semaphore");
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);  //delay
  }
}

void ConsumerTask(void *parameter) {
  while (1) {
    if (xSemaphoreTake(countingSemaphore, portMAX_DELAY) == pdTRUE) {
      Serial.println("Consumer took the semaphore");
       ledcWrite(2, ldr_state);
      Serial.println(ldr_state);

    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(led_pin, OUTPUT);
  pinMode(ldr_pin, INPUT);

  ledcAttach(led_pin, 5000, 10);  // Set PWM frequency and resolution

  countingSemaphore = xSemaphoreCreateCounting(1, 0);


  xTaskCreatePinnedToCore(
    ProducerTask,
    "Read LDR",
    1024,
    NULL,
    1,
    NULL,
    app_cpu);

  xTaskCreatePinnedToCore(
    ConsumerTask,
    "Brightness control",
    1024,
    NULL,
    1,
    NULL,
    app_cpu);

  vTaskDelete(NULL);

  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}
