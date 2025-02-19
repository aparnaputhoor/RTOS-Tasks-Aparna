#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static int global_var = 1;

static SemaphoreHandle_t xSemaphore;

void Task_A(void *parameter) {
  while (1) {
    if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {                       //task A to give semaphore
      Serial.println("Task A is takes the semaphore");
      if (global_var) {
        global_var = 0;
        Serial.print("global variable is made ");
        Serial.println(global_var);
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

void Task_B(void *parameter) {
  while (1) {

   if (xSemaphoreTake(xSemaphore, 0));
    else
    {
       global_var = 1;
    Serial.print("global variable is made ");
    Serial.println(global_var);
Serial.println("Task B is done. Giving semaphore to Task A.");
    xSemaphoreGive(xSemaphore);                                                  //Task B to give semaphore
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


void setup() {

  Serial.begin(115200);

  xSemaphore = xSemaphoreCreateBinary();  //created the semaphore

  xTaskCreatePinnedToCore(Task_A, "make it 0", 2048, NULL, 1, NULL,app_cpu);
  xTaskCreatePinnedToCore(Task_B, "make it 1", 2048, NULL, 1, NULL,app_cpu);
  vTaskDelete(NULL);
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}
