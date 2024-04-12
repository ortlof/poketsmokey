#ifndef AirStateService_h
#define AirStateService_h

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

#define DEFAULT_LED_STATE false
#define OFF_STATE "OFF"
#define ON_STATE "ON"

#define AIR_SETTINGS_ENDPOINT_PATH "/rest/airState"
#define AIR_SETTINGS_SOCKET_PATH "/ws/airState"

class AirState
{
public:
    bool airOn;
    bool coilOn;
    bool intervalOn;
    uint8_t airpower = 0;
    uint8_t coilpower = 0;
    uint8_t interval = 0;
    uint8_t burntime = 0;

    static void read(AirState& state, JsonObject& root)
    {
       root["air"] = state.airOn;
       root["airpower"] = state.airpower;
       root["coil"] = state.coilOn;
       root["coilpower"] = state.coilpower;
       root["intervalstate"] = state.intervalOn;
       root["interval"] = state.interval;
       root["burntime"] = state.burntime;
    }

    static StateUpdateResult update(JsonObject &root, AirState &airState)
    {
    airState.airOn = root["air"] | false;
    airState.coilOn = root["coil"] | false;
    airState.intervalOn = root["intervalstate"] | false;
    airState.airpower = root["airpower"] | 0;
    airState.coilpower = root["coilpower"] | 0;
    airState.interval = root["interval"] | 0;
    airState.burntime = root["burntime"] | 0;
    return StateUpdateResult::CHANGED;
    }


    static StateUpdateResult haUpdate(JsonObject &root, AirState &airState)
    {
        String state = root["state"];
        // parse new led state
        boolean newState = false;
        if (state.equals(ON_STATE))
        {
            newState = true;
        }
        else if (!state.equals(OFF_STATE))
        {
            return StateUpdateResult::ERROR;
        }
        // change the new state, if required
        if (airState.airOn != newState)
        {
            airState.airOn = newState;
            return StateUpdateResult::CHANGED;
        }
        return StateUpdateResult::UNCHANGED;
    }
};

class AirStateService : public StatefulService<AirState>
{
public:
    AirStateService(PsychicHttpServer *server,
                      SecurityManager *securityManager);
    void begin();

private:
    HttpEndpoint<AirState> _httpEndpoint;
    WebSocketServer<AirState> _webSocketServer;

    void registerConfig();
    void onConfigUpdated();
};
#endif
