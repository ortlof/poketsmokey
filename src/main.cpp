/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <ESP32SvelteKit.h>
#include <AirStateService.h>
#include <PowerService.h> 


#define SERIAL_BAUD_RATE 115200

bool ledState = 0;
const int haze = 10;
const int airpin = 11;
const int coilpin = 12;
float voltage;
float coilpw;
bool coilState = false;
bool airState = false;
float coilpowerset = 0;
float airpowerset = 0;
bool lowpower = true;
float burntimeset = 0; 
float burntimeinsek = 0;
float intervaltimeinsek = 0;
float intervalset = 0;
bool timerstate = false;
float maxburntimeset = 6000;
unsigned long burnStartMillis = 0;
unsigned long intervalStartMillis = 0;
bool burnInProgress = false;
bool intervalInProgress = false;


unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
unsigned long coilprevMillis;
const unsigned long period = 1500;  //the value is a number of milliseconds
const byte ledPin = 13;    //using the built in LED
const int coiltype = 0;

// setting PWM properties
const int freq = 15000;
const int airchannel = 0;
const int coilchannel = 1;
const int powerchannel = 3;
const int resolution = 8;

const int maxAmpere = 6; // Maximale Amperewert, an dem die Reduzierung der PWM startet
const int minPwm = 30; // Minimum PWM Ausgabe
const int maxPwm = 255; // Maximum PWM Ausgabe


AsyncWebServer server(80);
ESP32SvelteKit esp32sveltekit(&server);
AirStateService airStateService = AirStateService(&server,esp32sveltekit.getSecurityManager());
PowerStateService powerStateService = PowerStateService(&server,esp32sveltekit.getSecurityManager());

float getAnalogAverage(int pinNumber, int samples)
{
  float sum = 0;
  float average = 0;
  for (int i = 0; i < samples; i++)
  {
    // TODO: Possibly use fancier filters?
    sum += analogRead(pinNumber);
  }
  average = sum / samples;
  return average;
}

void battery(){
  voltage = getAnalogAverage(1, 20);
  //Serial.print(" ");
  //Serial.print(voltage);
  //voltage = analogRead(1);
  voltage = voltage * 10/4096;
  if(voltage < 3.5){
    lowpower = true;
   /* Serial.print(" ");
    Serial.print(voltage);
    Serial.println("Low Power");*/
  } else {
    lowpower = false;
    /*Serial.print(" ");
    Serial.print(voltage);
    Serial.println("High Power");*/
  }
  
  //Serial.print("Voltage= ");
  //Serial.println(voltage);
}

void coilpower() {
  static unsigned int sampleCount = 0; // count the number of samples
  static float accumulatedSamples = 0.0; // accumulate the sample values

  const unsigned long SAMPLE_INTERVAL_MS = 3;
  const unsigned int MAX_SAMPLES = 150;
  const unsigned int SENSOR_PIN = 18;
  const float OFFSET_VOLTAGE = 1.65;
  const float ADC_RESOLUTION = 4096.0;
  const float ARDUINO_VOLTAGE = 3.3;
  const float VOLTAGE_PER_AMPERE = 0.185;

  unsigned long currentMillis = millis(); // get the current time
  static unsigned long prevMillis = 0;

  if (currentMillis - prevMillis >= SAMPLE_INTERVAL_MS) {
    prevMillis = currentMillis; // update the previous time
    if(sampleCount < MAX_SAMPLES) { // get 150 samples
      float sensorValue = analogRead(SENSOR_PIN); // read current sensor values
      accumulatedSamples += sensorValue; // add samples together
      sampleCount++; // increment the sample count
    }
  }

  if (sampleCount == MAX_SAMPLES) { // if we've got all our samples
    float averageSample = accumulatedSamples / MAX_SAMPLES; // calculate the average

    // calculate the power of the coil
    coilpw = (OFFSET_VOLTAGE - (averageSample * (ARDUINO_VOLTAGE / ADC_RESOLUTION))) / VOLTAGE_PER_AMPERE;

    // reset the sample count and the accumulated samples for the next set
    sampleCount = 0;
    accumulatedSamples = 0.0;
  }
}

void coilburn(){
  if (coilState == true){
    int desiredPwm = map(coilpowerset, 0, 100, minPwm, maxPwm); // Mappe den gewünschten Prozentwert auf einen PWM-Wert
    ledcWrite(coilchannel, desiredPwm);
    } else {
    ledcWrite(coilchannel, 0);   
  }
}

void airpower(){
    if(airState == true){
      int desirePwm = map(airpowerset, 0, 100, 110, 255);
      ledcWrite(airchannel, desirePwm);
    } else {
      ledcWrite(airchannel, 0);
    };
}

void setup()
{
    
    pinMode(coilpin, OUTPUT);
    pinMode(airpin, OUTPUT);
    pinMode(21, OUTPUT);
    pinMode(1, INPUT);
    battery();
    ledcSetup(coilchannel, freq, resolution);
    ledcSetup(airchannel, freq, resolution);
    ledcSetup(powerchannel, freq, resolution);
    ledcAttachPin(coilpin, coilchannel);  
    ledcAttachPin(airpin, airchannel);
    ledcAttachPin(21, powerchannel);
    ledcWrite(powerchannel, 255);

    coilpower();

    // start serial and filesystem
    Serial.begin(SERIAL_BAUD_RATE);
    // start the framework and demo project
    esp32sveltekit.begin();
    // load the initial light settings
    airStateService.begin();
    powerStateService.begin();

    // start the server
    server.begin();
}


void loop() {
  // run the framework's loop function
  esp32sveltekit.loop();
  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - startMillis >= period)  //test whether the period has elapsed
  {
    battery();
    startMillis = currentMillis;  //IMPORTANT to save the start time of the current LED state.
    powerStateService.update([&](PowerState& state) {
      state.voltageState = voltage;
      state.coilpwState = coilpw;
      return StateUpdateResult::CHANGED;
    }, "Bat/Coil-Timer");
  }

  if (timerstate == true) { //Fehler hier.
    burntimeinsek = burntimeset * 1000; 
    intervaltimeinsek = intervalset * 1000;
    
    if (burnInProgress == false && intervalInProgress == false) {
      burnStartMillis = currentMillis;
      burnInProgress = true;
      airState = true;
      coilState = true;
      Serial.println("Timer On");
    }
    
    if (burnInProgress == true && currentMillis - burnStartMillis >= burntimeinsek) {
      burnInProgress = false;
      intervalInProgress = true;
      intervalStartMillis = currentMillis;
      airState = false;
      coilState = false;
      Serial.println("Timer Off");
    }
    
    if (intervalInProgress == true && currentMillis - intervalStartMillis >= intervaltimeinsek) { // only divide by 1000 once
      intervalInProgress = false;
      Serial.println("Wait ");
    }
  } 
  
  if (lowpower == false) {
    coilpower();
    coilburn();
    airpower();
  } else {
    ledcWrite(coilchannel, 0);
  }
}


