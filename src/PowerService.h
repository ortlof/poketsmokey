#ifndef PowerService_h
#define PowerService_h

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
#include <HttpEndpoint.h>
#include <WebSocketServer.h>
#include <PsychicHttp.h>
#include <ArduinoJson.h>

#define MAX_ESP_STATUS_SIZE 1024
#define POWER_SETTINGS_ENDPOINT_PATH "/rest/powerState"
#define POWER_SETTINGS_SOCKET_PATH "/ws/powerState"

extern float voltage;
extern float coilpw;

class PowerState
{
public:

   float voltageState = voltage;
   float coilpwState = coilpw;

    static void read(PowerState& state, JsonObject& root)
    {
       root["voltage"] = state.voltageState;
       root["coilpw"] = state.coilpwState;
    }

    static StateUpdateResult update(JsonObject &root, PowerState &powerState)
    {
    powerState.voltageState = root["voltage"];
    powerState.coilpwState = root["coilpw"];
    return StateUpdateResult::CHANGED;
    }

};

class PowerStateService : public StatefulService<PowerState>
{
public:
    PowerStateService(PsychicHttpServer *server,
                      SecurityManager *securityManager);
    void begin();

private:
    HttpEndpoint<PowerState> _httpEndpoint;
    WebSocketServer<PowerState> _webSocketServer;

    void registerConfig();
    void onConfigUpdated();
};

#endif
