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
#include <WebSocketTxRx.h>

#define DEFAULT_LED_STATE false
#define OFF_STATE "OFF"
#define ON_STATE "ON"

#define AIR_SETTINGS_ENDPOINT_PATH "/rest/airState"
#define AIR_SETTINGS_SOCKET_PATH "/ws/airState"

class AirState
{
public:
    bool airOn;
    uint8_t power = 0;

    static void read(AirState& state, JsonObject& root)
    {
       root["air"] = state.airOn;
       root["power"] = state.power;
    }

    static StateUpdateResult update(JsonObject &root, AirState &airState)
    {
    airState.airOn = root["air"] | false;
    airState.power = root["power"] | 255;
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
    AirStateService(AsyncWebServer *server,
                      SecurityManager *securityManager);
    void begin();

private:
    HttpEndpoint<AirState> _httpEndpoint;
    WebSocketTxRx<AirState> _webSocket;

    void registerConfig();
    void onConfigUpdated();
};
#endif
