#pragma once
#include <SDL3/SDL.h>
#include "../include/core.h"
#include "../include/appstate.h"

namespace core {


    typedef SDL_Event Event;

    float GetDelta();
    SDL_Event* GetEvent();
    bool appDone();
    int on_init(void(*func)());
    int on_event(void(*func)());
    int on_update(void(*func)());
    void on_quit();
    AppState* GetAppState();


}
