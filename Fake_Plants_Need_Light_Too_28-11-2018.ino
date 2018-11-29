/*
 * 
 * --FAKE PLANTS NEED LIGHT TOO --
 * 
 * November 28, 2018
 * 
 *         
 *    \| 
 *     |/
 *    \| 
 *     |/
 *    \| 
 *     |/
 *_____|_____
 * 
 * 
 * 'Fake Plants Need Light Too' is a project by Robert Pearce, Chun-Wei Lee, 
 * and Hazar Gizem Peker.
 * 
 * The code below operates a fake plant that uses shape memory alloy and 
 * thermochromic paint to mimic dying in the presence of low light and 
 * coming back to life in the presence of bright light. The code also operates
 * an 8x8 LED dot matrix that displays whether the plant is in 'dying' mode,
 * 'growing' mode, or 'calibrating' mode (the mode in which the photoresistor
 * is calibrated to the ambient light in the room when the device is turned on).
 * 
 * The 'SETTINGS' section below enables the programmer to alter the following:
 *  - The brightness of the display
 *  - The 'growth' time of the plant (as controlled by the duty cycle of the
 *    PWM signal controlling the plant)
 *  - The threshold for the relative brightness change required to activate the  
 *    plant
 * 
 * 
 */

//Include library for controlling 8x8 dot-matrix display
#include "LedControl.h"

//---SETTINGS---
int displayBrightness = 1;//Brightness of the display (0 to 15)
int dutyCyclePWM = 50;//Duty cycle of PWM pin to mosfet (0 to 255)
int lightDeviationThreshold = 25; //Threshold for light sensor, the higher the threshold the less sensitive the plant will be to light change (0 to 1024)

//Define pins for the display
const int DIN_PIN = 11;
const int CS_PIN = 10;
const int CLK_PIN = 9;

//Define integer for holding light sensor calibration values 
int lightSensorCalibrationAverage = 0;

//Define pins for the sensor and mosfet
int lightSensorPin = A0;
int lightSensorValue = 0;
int mosfetPin = 3;

//Define variables for counting in the for loops
int i = 0;
int y = 0;
int m = 0;
int n = 0;

//Define plant growing animation as a series of base 16 arrays
const uint64_t GROWING[] = {
  0x0000000000000000,
  0x0800000000000000,
  0x0808000000000000,
  0x0808080000000000,
  0x0808180800000000,
  0x0808183808000000,
  0x080818382c080000,
  0x080818382c0e0800,
  0x080818382c0e0a08,
  0x080818382c0e0a08,
  0x080818382c0e0a08,
  0x080818382c0e0a08,
  0x080818382c0e0a08,
  0x080818382c0e0a08,
  };
  
//Define length of growing animation
const int GROWING_LEN = sizeof(GROWING) / sizeof(uint64_t);

//Define plant dying animation as a series of base 16 arrays
const uint64_t DYING[] = {
  0x080818382c0e0a08,
  0x080818382c0e0a08,
  0x080818382c0e0a08,
  0x080818382c0e0a08,
  0x080818382c0e0a08,
  0x080818382c0e0a08,
  0x080818780c0f0810,
  0x080878280f0a1020,
  0x080878280f0a1020,
  0x0868380b0e906000,
  0x28383a0e8e700000,
  0x28381a8e4c300000,
  0x28388a4e30000000,
  0x28ba4e3000000000 
  };

//Define length of dying animation
const int DYING_LEN = sizeof(DYING) / sizeof(uint64_t);

//Define the calibration animation as a series of base 16 arrays
const uint64_t CALIBRATING[] = {
  0x0018244242241000,
  0x0018244242240800,
  0x0018244242041800,
  0x0018244202241800,
  0x0018240242241800,
  0x0018044242241800,
  0x0008244242241800,
  0x0010244242241800,
  0x0018204242241800,
  0x0018244042241800,
  0x0018244240241800,
  0x0018244240241800,
  0x0018244242201800
  };

//Establish LedControl library settings by indicating which pins connect to the display
LedControl display = LedControl(DIN_PIN, CLK_PIN, CS_PIN);



void setup() {
  //Clear display and set brightness
  display.clearDisplay(0);
  display.shutdown(0, false);
  display.setIntensity(0, displayBrightness);

  //Establish the initial mode of the pins
  pinMode(lightSensorPin, INPUT);
  pinMode(mosfetPin, OUTPUT);
  digitalWrite(mosfetPin, LOW);

  //Establish a baseline reading for the light sensor to compare against by taking twelve readinss over 3.6 seconds and averaging them
  //Also, play one frame of the 'CALIBRATING' animation for each reading taken
  y=0;
  for (int y = 0; y <= 12; y++){
    for (int i = 0; i < 8; i++) {
      byte row = (CALIBRATING[y] >> i * 8) & 0xFF;
      for (int j = 0; j < 8; j++) {
        display.setLed(0, i, j, bitRead(row, j));
      }
    }
    lightSensorCalibrationAverage = lightSensorCalibrationAverage + analogRead(lightSensorPin);
    delay(300);
  }
  //Average the twelves calibration sensor readings to get a usable value
  lightSensorCalibrationAverage = lightSensorCalibrationAverage/12;
}



//Function for writing animations to the display by turning on or off each pixel individually
//This function is not used for the CALIBRATION animation
void displayImage(uint64_t image) {
  for (int i = 0; i < 8; i++) {
    byte row = (image >> i * 8) & 0xFF;
    for (int j = 0; j < 8; j++) {
      display.setLed(0, i, j, bitRead(row, j));
    }
  }
}



void loop() {
  lightSensorValue = analogRead(lightSensorPin); //Read light sensor value
  
  /*If the current light sensor value is higher than the calibration average 
   * by the specified threshold, then play the GROWING animation and activate the mosfet.
   * Otherwise, play the DYING animation and do not activate the mosfet.*/
  if (lightDeviationThreshold < lightSensorValue - lightSensorCalibrationAverage) {
    m = 0;
    analogWrite(mosfetPin, dutyCyclePWM); 
    displayImage(GROWING[n]);
    if (++n >= GROWING_LEN ) {
      n = 0;
    }
  } else {
    n = 0;
    digitalWrite(mosfetPin, LOW);
    displayImage(DYING[m]);
    if (++m >= DYING_LEN ) {
      m = 0;
  }
  }
  //Set the cycle time of each loop
  delay(150);
}
