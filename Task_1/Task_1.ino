#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const int led_pin = 2;
static const int button_pin = 4;

static SemaphoreHandle_t mutex;
static int led_state = 0;

void toggleLED(void *parameter) {
  while (1) {
    if (led_state) {                              //if state is high
      digitalWrite(led_pin, HIGH);                //led on
    } else {
      digitalWrite(led_pin, LOW);                 //if state is low led is off
    }
  }
}

void Pushbutton(void *parameter) {
  while (1) {

    if (digitalRead(button_pin)) {                  //if button is pressed
       xSemaphoreTake(mutex, portMAX_DELAY);        //take mutex
      led_state = !led_state;                       //toggle the state
       xSemaphoreGive(mutex);                       //give mutex
    vTaskDelay(200/ portTICK_PERIOD_MS);            //for button debouncing
    }
  }
}


void setup() {

  Serial.begin(115200);

  pinMode(led_pin, OUTPUT);
  pinMode(button_pin, INPUT);

  mutex = xSemaphoreCreateMutex();        //created the semaphore


  xTaskCreatePinnedToCore(
    toggleLED,
    "Toggle LED",
    1024,
    NULL,
    1,
    NULL,
    app_cpu);

  xTaskCreatePinnedToCore(
    Pushbutton,
    "Push Button",
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
