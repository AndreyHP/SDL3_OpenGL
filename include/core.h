#pragma once

#include "../include/core.h"
#include "../include/imgui/imgui.h"
#include "../include/imgui/backends/imgui_impl_sdl3.h"
#include "../include/imgui/backends/imgui_impl_opengl3.h"


namespace core {

    bool appDone();
    int on_init();
    int on_event();
    int on_update();
    void on_quit();

}
