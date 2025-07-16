#include "../include/core.h"

AppState *appstate = core::GetAppState();

void teste(){
   const double delta = core::GetDelta();

   core::Event *event = core::GetEvent();


   if (event->type == SDL_EVENT_KEY_DOWN) {
        switch ((int)event->key.scancode) {

        case SDL_SCANCODE_W:
            appstate->camera.ProcessKeyboard(FORWARD, delta);
            break;
        case SDL_SCANCODE_A:
            appstate->camera.ProcessKeyboard(LEFT, delta);
            break;
        case SDL_SCANCODE_S:
            appstate->camera.ProcessKeyboard(BACKWARD, delta);
            break;
        case SDL_SCANCODE_D:
            appstate->camera.ProcessKeyboard(RIGHT, delta);
            break;
        case SDL_SCANCODE_O:
           // models[0].outline = !models[0].outline;
            break;
        case SDL_SCANCODE_P:
           // postprecess = !postprecess;
            break;
        case SDL_SCANCODE_F:
            appstate->wireframe = !appstate->wireframe;
            break;
        case SDL_SCANCODE_U:
            //for (int i = 0; i < models.size(); i++) {
            //models[i].unload();
           // }
            break;
        }
        }
}


int main(){

 core::on_init();

 while (!core::appDone()) {
    core::on_event(&teste);
    core::on_update();
    }

 core::on_quit();

    return 0;
}
