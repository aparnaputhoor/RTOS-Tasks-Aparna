/*
Use a binary semaphore to synchronize an interrupt service routine (ISR) with a task. 
In this project, a push button generates an interrupt, and the ISR "gives" the semaphore.
A task waiting on the semaphore then toggles an LED.

How It Works:
ISR: On a button press, use xSemaphoreGiveFromISR().
Task: Continuously call xSemaphoreTake(semaphore, portMAX_DELAY) 
and, once the semaphore is received, toggle the LED.
*/


// #if CONFIG_FREERTOS_UNICORE
// static const BaseType_t app_cpu = 0;
// #else
// static const BaseType_t app_cpu = 1;
// #endif

static const int led_pin = 2;
static const int button_pin = 4;

static SemaphoreHandle_t binarySemaphore;

static int led_state = 0;

void toggleLED(void *parameter) {
  while (1) {
     if (xSemaphoreTake(binarySemaphore, portMAX_DELAY) == pdTRUE) {
    if (led_state) {                              //if state is high
      digitalWrite(led_pin, HIGH);                //led on
    } else {
      digitalWrite(led_pin, LOW);                 //if state is low led is off
    }
     vTaskDelay(200/ portTICK_PERIOD_MS); 
  }
  }
}

void buttonISR( ) {
      xSemaphoreGive(binarySemaphore);
      led_state = !led_state;                       //toggle the state 
  }


void setup() {

  Serial.begin(115200);

  pinMode(led_pin, OUTPUT);
  pinMode(button_pin, INPUT);

  binarySemaphore = xSemaphoreCreateBinary();        //created the semaphore

  attachInterrupt(button_pin, buttonISR, FALLING);

  xTaskCreate(
    toggleLED,
    "Toggle LED",
    2048,
    NULL,
    1,
    NULL);
  // put your setup code here, to run once:
  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}
