#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif


QueueHandle_t xQueue;


#define button_1 4
#define button_2 5
#define button_3 21

#define led_1 12
#define led_2 14
#define led_3 26

int data = 0;

void readserialmonitor(void *parameter) {
  while (1) {
    if (Serial.available() > 0) {  // Check if data is available
      data = Serial.parseInt();    // Read an integer
      if (data >= 1 && data <= 3) {
        Serial.println(data);                      // Print the entered value
        xQueueSend(xQueue, &data, portMAX_DELAY);  //send the value in queue
      } else {
        Serial.println("Invalid input, please enter a valid number.");
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);  // Delay for 2 seconds before
  }
}

void readbutton(void *parameter) {
  while (1) {
    data = 0;
    if (digitalRead(button_1) == LOW) {  //read the button press and make the data 1 2 3 to turn on corresponding led
      data = 1;
    }
    if (digitalRead(button_2) == LOW) {
      data = 2;
    }
    if (digitalRead(button_3) == LOW) {
      data = 3;
    }
    if (data >= 0) {  //if it is the correct data send it
      if (xQueueSend(xQueue, &data, portMAX_DELAY) == pdTRUE)
        ;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);  // Delay
  }
}

void printData(void *parameter) {
  while (1) {
    if (xQueueReceive(xQueue, &data, portMAX_DELAY)) {  //receive the data
      if (data == 1) {
        digitalWrite(led_1, HIGH);  //turn on the corresponding led
      } else if (data == 2) {
        digitalWrite(led_2, HIGH);
      } else if (data == 3) {
        digitalWrite(led_3, HIGH);
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);  //turn on for 1sec

      digitalWrite(led_1, LOW);  //turn off all the led
      digitalWrite(led_2, LOW);
      digitalWrite(led_3, LOW);
    }
  }
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  pinMode(button_1, INPUT_PULLUP);
  pinMode(button_2, INPUT_PULLUP);
  pinMode(button_3, INPUT_PULLUP);

  pinMode(led_1, OUTPUT);
  pinMode(led_2, OUTPUT);
  pinMode(led_3, OUTPUT);

  xQueue = xQueueCreate(5, sizeof(int));

  xTaskCreatePinnedToCore(readbutton, "read the data", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(printData, "print the data", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(readserialmonitor, "read from serial monitor", 2048, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}
