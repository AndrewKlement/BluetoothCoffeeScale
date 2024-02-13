#include <Arduino.h>
#include "HX711.h"
#include "soc/rtc.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TM1640.h>           //LED matrix chip libary
#include <TM16xxMatrixGFX.h>  
#include <Fonts/Picopixel.h>
#include <string>
#include "BluetoothSerial.h"
#include <Preferences.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Bluetooth not available or not enabled.
#endif

// HX711
const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN = 4;
HX711 scale;

//LED Matrix
TM1640 module(21, 22);    // I2C connection for LED matrix
#define MODULE_SIZECOLUMNS 16    // number of GRD lines, will be the y-height of the display
#define MODULE_SIZEROWS 8    // number of SEG lines, will be the x-width of the display
TM16xxMatrixGFX matrix(&module, MODULE_SIZECOLUMNS, MODULE_SIZEROWS);    // TM16xx object, columns, rows

//BluetoothSerial
BluetoothSerial SerialBT;

//flash memory
Preferences preferences;

//variable
float last_batteryLevel = 0.0;
bool start_rec = false; 
float lastReading = 0.0;
float avarageScale = 0;
int counter = 0;
float reading = 0;
bool display_text = false; 

void displayText(String text){
  matrix.fillScreen(LOW); 
  matrix.write();
  matrix.setCursor(0,7);
  matrix.print(text);
  matrix.write();
}

void setup() {
  Serial.begin(115200);
  
  // Scale Begin
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  scale.set_scale(230.9026);
  scale.tare(); 
  //LED Matrix 8x16
  matrix.setIntensity(5); // Use a value between 0 and 7 for brightness
  matrix.setRotation(1);
  matrix.setMirror(true); 
  matrix.setFont(&Picopixel);
  
  //Bluetooth Begin
  SerialBT.begin("Scale");

  // Scale begin at 0 g
  displayText("CON");
  SerialBT.print(String(0)); 
}

void loop() {
  if (SerialBT.available()) {
    String bt_reading = SerialBT.readString();
    if (bt_reading == "tare") {
      scale.tare(); 
    } 
    if (bt_reading == "calibrate") {
      scale.set_scale();
      displayText("CLR");
      delay(4000);
      scale.tare();
      delay(4000); 
      displayText(String(500));
      delay(4000); 
      int reading = scale.get_units(40);
      int calibration_factor = reading/500; 
      scale.set_scale(calibration_factor);
      preferences.begin("factor", false);
      preferences.putFloat("cal_factor", calibration_factor);
      preferences.end();
      delay(4000); 
    }        
  }  
  float foo = scale.get_units(15); 
  int reading = round(foo); 
  Serial.println(reading);
  if (lastReading != reading){
    if (reading >= 0){
        lastReading = reading;
        displayText(String(reading));
        Serial.println(String(reading));
        SerialBT.print(String(reading)); 
    }
    else{
      displayText(String(0));
      Serial.println(String(reading));
      SerialBT.print(String(0)); 
    } 
  }
}
