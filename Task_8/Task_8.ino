#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static int value = 0;


#define LED1_PIN 12  // Pin for LED 12
#define LED2_PIN 13  // Pin for LED 13
#define LED3_PIN 14  // Pin for LED 14

int led_pins[]={LED1_PIN, LED2_PIN, LED3_PIN};

static SemaphoreHandle_t countingSemaphore;

void setup() {

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);

  countingSemaphore = xSemaphoreCreateCounting(2, 2);

  xTaskCreatePinnedToCore(
    Task1,
    "led pattern 1",
    1024,
    NULL,
    1,
    NULL,
    app_cpu);

  xTaskCreatePinnedToCore(
    Task2,
    "led pattern 2",
    1024,
    NULL,
    1,
    NULL,
    app_cpu);

  xTaskCreatePinnedToCore(
    MasterTask,
    "pattern control",
    1024,
    NULL,
    1,
    NULL,
    app_cpu);
}

void loop() {
}


void MasterTask(void *parameter) {
  while (1) {
    xSemaphoreGive(countingSemaphore);            //give the semaphore every 2 sec
    vTaskDelay(1500 / portTICK_PERIOD_MS);
  }
}

void Task2(void *parameter) {
  while (1) {
    if (xSemaphoreTake(countingSemaphore, portMAX_DELAY) == pdTRUE) {

      for (int i = 0; i < 3; i++) {
        digitalWrite(led_pins[i], HIGH);           //turn on one by one     
        for (int j = 0; j < 3; j++) {
          if (i != j) {
            digitalWrite(led_pins[j], LOW);             //off led other than the one on
          }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);  // Wait 100ms
        digitalWrite(led_pins[i], LOW);         //off the current led
      }
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void Task1(void *parameter) {
  while (1) {
    if (xSemaphoreTake(countingSemaphore, portMAX_DELAY) == pdTRUE) {

      for (int i = 0; i < 3; i++) {
        digitalWrite(led_pins[i], HIGH);
        for (int j = 0; j < 3; j++) {
          if (i != j) {
            digitalWrite(led_pins[j], LOW);
          }
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);           //turn on and off with more delay
        digitalWrite(led_pins[i], LOW);
      }
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
