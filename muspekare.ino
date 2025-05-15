
#include "driver/rtc_io.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#include <BleMouse.h>

BleMouse bleMouse;

#include <math.h>

#define BUTTON_LEFT 1
#define BUTTON_SPECIAL 3
#define BUTTON_RIGHT 2

Adafruit_MPU6050 mpu;

unsigned long sleepTimer = 0;


float senses[] = {1.0,1.4,1.8,2.2,2.6,3.0,3.4,3.8};
int sensIndex = 4;

void setup() {
  bleMouse.end();
  bleMouse.begin();
  while(!bleMouse.isConnected()){
        delay(1000);
    }

  findMpu();
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
 
  pinMode(BUTTON_LEFT, INPUT_PULLUP);  
  pinMode(BUTTON_SPECIAL, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  
  gpio_wakeup_enable(GPIO_NUM_1, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();

  sleepTimer = millis();
}



int loopDelay = 0;

unsigned long startTime = 0;

float mousesens = 2;

void loop() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    if (digitalRead(BUTTON_LEFT) == LOW) {
        bleMouse.press(MOUSE_LEFT);
        resetSleepTimer();
    }
    else{
        bleMouse.release(MOUSE_LEFT);
    }

    if (digitalRead(BUTTON_RIGHT) == LOW) {
        bleMouse.press(MOUSE_RIGHT);
        resetSleepTimer();
    }
    else{
        bleMouse.release(MOUSE_RIGHT);
    }
    
    float deltaX = g.gyro.z*a.acceleration.z + g.gyro.x*a.acceleration.x;
    float deltaY = g.gyro.x*a.acceleration.z - g.gyro.z*a.acceleration.x;
    deltaX = deltaX/sqrt(a.acceleration.z*a.acceleration.z +a.acceleration.x*a.acceleration.x);
    deltaY = deltaY/sqrt(a.acceleration.z*a.acceleration.z +a.acceleration.x*a.acceleration.x);


    if(!(deltaX<0.1 && deltaX > -0.1) || !(deltaX<0.1 && deltaX > -0.1)){
        resetSleepTimer();
    }
    
    float deltaTime = (millis() - startTime);
    mousesens = senses[sensIndex];
    bleMouse.move(-mousesens*(deltaX)*deltaTime, -mousesens*deltaY*deltaTime);
    startTime = millis();

    if( millis() > sleepTimer + 20000){
      sleep();
    }

    specialButton();
    delay(loopDelay);
}

void findMpu(){
  if (!mpu.begin(0x68)) {
    while (1) {
      delay(10);
    }
  }
}

void specialButton(){
  if(digitalRead(BUTTON_SPECIAL) != LOW){
    return;
  }
  
  unsigned long temp = millis();
  while(digitalRead(BUTTON_SPECIAL) == LOW){
      //Stals here 
  }

  if(millis() - temp < 1000){
    toggleSenseMode();
  }

  startTime = millis();
  
  
}

void toggleSenseMode(){
    sensIndex++;
    sensIndex = sensIndex % 6;
}

void resetSleepTimer(){
  sleepTimer = millis();
}

void sleep(){
    esp_light_sleep_start();
    esp_restart();
}
