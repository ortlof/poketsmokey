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
extern float voltage;
extern float coilpw;
extern bool coilState;
extern bool airState; 
extern bool timerstate;
extern float airpowerset;
extern float coilpowerset;
extern float burntimeset;
extern float intervalset;

const int burntime = 0;
const int interval = 0;
bool airOn = false;



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
    // configure settings service update handler to update LED state
    addUpdateHandler([&](const String &originId){ 
      onConfigUpdated();},
                     false);
}

void AirStateService::begin()
{
    _state.airOn = DEFAULT_LED_STATE;
    _state.coilOn = DEFAULT_LED_STATE;
    _state.intervalOn = DEFAULT_LED_STATE;
    _state.airpower = 0;
    _state.coilpower = 0;
    _state.burntime = 0;
    _state.interval = 0;
    onConfigUpdated();
}

void AirStateService::onConfigUpdated()
{
    airState  = _state.airOn;
    coilState = _state.coilOn;
    coilpowerset = _state.coilpower;
    airpowerset = _state.airpower;
    burntimeset = _state.burntime;
    intervalset = _state.interval;
    timerstate = _state.intervalOn;
}