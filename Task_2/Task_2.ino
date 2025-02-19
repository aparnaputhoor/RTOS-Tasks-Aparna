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

static SemaphoreHandle_t mutex;
static int ldr_state = 0;

void LEDControl(void *parameter) {
  while (1) {
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      //analogWrite(led_pin, ldr_state);
      ledcWrite(led_pin, ldr_state);  //pwm to pin 4
      xSemaphoreGive(mutex);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void readLDR(void *parameter) {
  while (1) {
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {  //take mutex
      ldr_state = analogRead(ldr_pin) >> 4;      //analog value from ldr sensor
      Serial.println(ldr_state);
      xSemaphoreGive(mutex);  //give mutex
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);  //delay
  }
}


void setup() {

  Serial.begin(115200);

  pinMode(led_pin, OUTPUT);
  pinMode(ldr_pin, INPUT);

  ledcAttach(led_pin, 5000, 10);  // Set PWM frequency and resolution

  mutex = xSemaphoreCreateMutex();  //created the semaphore


  xTaskCreatePinnedToCore(
    readLDR,
    "Read LDR",
    1024,
    NULL,
    1,
    NULL,
    app_cpu);

  xTaskCreatePinnedToCore(
    LEDControl,
    "Brightness control",
    1024,
    NULL,
    2,
    NULL,
    app_cpu);
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}
