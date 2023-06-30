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

#include <AirStateService.h>

const int airpin = 11;

// setting PWM properties
const int freq = 15000;
const int airchannel = 0;
const int resolution = 8;

AirStateService::AirStateService(AsyncWebServer *server,
                                 SecurityManager *securityManager) : _httpEndpoint(AirState::read,
                                                                                   AirState::update,
                                                                                   this,
                                                                                   server,
                                                                                   AIR_SETTINGS_ENDPOINT_PATH,
                                                                                   securityManager,AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                   _webSocket(AirState::read,
                                                                                   AirState::update,
                                                                                   this,
                                                                                   server,
                                                                                   AIR_SETTINGS_SOCKET_PATH,
                                                                                   securityManager,AuthenticationPredicates::IS_AUTHENTICATED)
{
    // configure led to be output
    pinMode(airpin, OUTPUT);
    ledcSetup(airchannel, freq, resolution);
    ledcAttachPin(airpin, airchannel);

    // configure settings service update handler to update LED state
    addUpdateHandler([&](const String &originId){ 
      onConfigUpdated();
      Serial.print("The light's state has been updated by: ");},
                     false);
}

void AirStateService::begin()
{
    _state.airOn = DEFAULT_LED_STATE;
    _state.power = 0;
    onConfigUpdated();
}

void AirStateService::onConfigUpdated()
{
    if(_state.airOn == true){
      ledcWrite(airchannel, _state.power);
    } else {
      ledcWrite(airchannel, 0);
    };
}