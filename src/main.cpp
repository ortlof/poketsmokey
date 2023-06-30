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
const int hazepin = 10;
const int airpin = 11;
const int smokepin = 12;
float voltage;
float coilpw;

unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 1500;  //the value is a number of milliseconds
const byte ledPin = 13;    //using the built in LED


AsyncWebServer server(80);
ESP32SvelteKit esp32sveltekit(&server);
AirStateService airStateService = AirStateService(&server,esp32sveltekit.getSecurityManager());
PowerStateService powerStateService = PowerStateService(&server,esp32sveltekit.getSecurityManager());

float getAnalogAveragePercent(int pinNumber, int samples)
{
    float sum = 0;
    float average = 0;
    float percentage = 0;
    for (int i = 0; i < samples; i++)
    {
        // TODO: Possibly use fancier filters?
        sum += analogRead(pinNumber);
    }
    average = sum / samples;
    // TODO: Might want to add a deadband
    percentage = 100.0 * average / 4096.0; // 12 bit resolution
    return percentage;
}

void battery(){
  voltage = analogRead(1);
  voltage = voltage * 10/4096;
  //Serial.print("Voltage= ");
  //Serial.println(voltage);
}

void coilpower(){
  unsigned int x=0;
  float AcsValue=0.0,Samples=0.0,AvgAcs=0.0,AcsValueF=0.0;

  for (int x = 0; x < 150; x++){ //Get 150 samples
  AcsValue = analogRead(18);     //Read current sensor values   
  Samples = Samples + AcsValue;  //Add samples together
  delay (3); // let ADC settle before next sample 3ms
  }
  AvgAcs=Samples/150.0;//Taking Average of Samples

//((AvgAcs * (5.0 / 1024.0)) is converitng the read voltage in 0-5 volts
//2.5 is offset(I assumed that arduino is working on 5v so the viout at no current comes
//out to be 2.5 which is out offset. If your arduino is working on different voltage than 
//you must change the offset according to the input voltage)
//0.185v(185mV) is rise in output voltage when 1A current flows at input
coilpw = (1.65 - (AvgAcs * (3.3 / 4096.0)) )/0.185;
//Serial.println(AcsValueF);//Print the read current on Serial monitor 
}

void setup()
{
    battery();
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

void loop()
{
    // run the framework's loop function
    esp32sveltekit.loop();
    currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
    if (currentMillis - startMillis >= period)  //test whether the period has elapsed
    {
      battery();
      coilpower();
      startMillis = currentMillis;  //IMPORTANT to save the start time of the current LED state.
     powerStateService.update([&](PowerState& state) {
      state.voltageState = voltage;
      state.coilpwState = coilpw;
     return StateUpdateResult::CHANGED;
      }, "Bat/Coil-Timer");
    }
}
