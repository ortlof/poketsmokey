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

#include <PowerService.h>
extern float voltage;
extern float coilpw;

PowerStateService::PowerStateService(PsychicHttpServer *server,
                                 SecurityManager *securityManager) : _httpEndpoint(PowerState::read,
                                                                                   PowerState::update,
                                                                                   this,
                                                                                   server,
                                                                                   POWER_SETTINGS_ENDPOINT_PATH,
                                                                                   securityManager,AuthenticationPredicates::IS_AUTHENTICATED),
                                                                    _webSocketServer(PowerState::read,
                                                                                   PowerState::update,
                                                                                   this,
                                                                                   server,
                                                                                   POWER_SETTINGS_SOCKET_PATH,
                                                                                   securityManager,AuthenticationPredicates::IS_AUTHENTICATED)
{
    // configure settings service update handler to update LED state
    addUpdateHandler([&](const String &originId){ 
      onConfigUpdated();},
                     false);
}

void PowerStateService::begin()
{

    _httpEndpoint.begin();
    _webSocketServer.begin();
    
    _state.voltageState = voltage;
    _state.coilpwState = coilpw;
    onConfigUpdated();
}

void PowerStateService::onConfigUpdated()
{
    _state.voltageState = voltage;
    _state.coilpwState = coilpw;
}
