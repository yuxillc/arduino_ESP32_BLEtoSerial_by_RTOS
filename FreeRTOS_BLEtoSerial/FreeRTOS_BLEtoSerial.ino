
#define BLERAM 32

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;


// define two tasks for Blink & AnalogRead
void TaskBLE( void *pvParameters );
void TaskSerial( void *pvParameters );
QueueHandle_t queue;
// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial communication at 115200 bits per second:
  Serial.begin(57600);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  queue = xQueueCreate( 32, sizeof( int ) );
  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
    TaskBLE
    ,  "TaskBLE"   // A name just for humans
    ,  1024  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskSerial
    ,  "TaskSerial"
    ,  1024  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBLE(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
    
  If you want to know what pin the on-board LED is connected to on your ESP32 model, check
  the Technical Specs of your board.
*/
  int ble;
  // initialize digital LED_BUILTIN on pin 13 as an output.

  for (;;) // A Task shall never return or exit.
  {
    for(int i = 0; i<BLERAM; i++){
      xQueueReceive(queue, &ble, portMAX_DELAY);
      if(ble!=0xff){SerialBT.write(ble);}
    }
    vTaskDelay(2);  // one tick delay (15ms) in between reads for stability
  }
}

void TaskSerial(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  
/*
  AnalogReadSerial
  Reads an analog input on pin A3, prints the result to the serial monitor.
  Graphical representation is available using serial plotter (Tools > Serial Plotter menu)
  Attach the center pin of a potentiometer to pin A3, and the outside pins to +5V and ground.

  This example code is in the public domain.
*/
  int serial;
  for (;;)
  {
    for(int i = 0; i<BLERAM; i++){
      serial=Serial.read();
      if(serial!=0xff){
        xQueueSend(queue, &serial , portMAX_DELAY);
      }
    }
    vTaskDelay(2);  // one tick delay (15ms) in between reads for stability
  }
}
