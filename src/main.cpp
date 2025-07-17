#include "../include/core.h"
#ifdef __EMSCRIPTEN__
#include "../include/emscripten_mainloop_stub.h"
#endif


AppState *appstate = core::GetAppState();
vector<Model> models;
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);


void init(){
    for (int i = 0; i < 1; i++) {
        Model newmodel;
        newmodel.id = i;
        models.push_back(newmodel);
    }

}

void render(){

    std::cout << appstate->modelLoaded << std::endl;
    // view/projection transformations
    projection = glm::perspective(
        glm::radians(appstate->camera.Zoom),
        (float)appstate->SCR_WIDTH / (float)appstate->SCR_HEIGHT, 0.1f, 100.0f);
    view = appstate->camera.GetViewMatrix();

    for (int i = 0; i < models.size(); i++) {
        if (models[i].outline) {
        models[i].OutlineInit(appstate->singleColorShader);
        }

        if (models[i].outline) {
        appstate->singleColorShader.setMat4("view", view);
        appstate->singleColorShader.setMat4("projection", projection);

        appstate->defaultShader.use();
        appstate->defaultShader.setMat4("projection", projection);
        appstate->defaultShader.setMat4("view", view);
        } else {
        appstate->defaultShader.use();
        appstate->defaultShader.setMat4("projection", projection);
        appstate->defaultShader.setMat4("view", view);
        }

        // render the loaded model
        if (appstate->showModel) {
        models[i].Draw(appstate->defaultShader);
        }

        // float newAngle = rotateModel * now;

        //models[i].Translate(0.0f + i * 2.0f, YTranslate, ZTranslate);
        // models[i].Rotate(0.0f, 0.5f, 0.0f, newAngle);
       // models[i].Scale(scale, scale, scale);
        appstate->defaultShader.setMat4("model", models[i].model);

        if (appstate->modelLoaded) {
        std::cout << "loaded" << std::endl;
        for (int i = 0; i < models.size(); i++) {
            models[i].unload();
            models[i].Load(appstate->inputBuffer);
        }
        appstate->modelLoaded = false;
        appstate->showModel = true;
        }

        if (models[i].outline) {

        models[i].drawOutline(appstate->singleColorShader);

        //models[i].Translate(0.0f + i * 2.0f, YTranslate, ZTranslate);
        // models[i].Rotate(0.0f, 0.5f, 0.0f, newAngle);
        //models[i].Scale(scale + 0.02f, scale + 0.02f, scale + 0.02f);
        }
    }
}

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

 core::on_init(&init);
 #ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!core::appDone())
#endif

  {
    core::on_event(&teste);
    core::on_update(&render);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

 core::on_quit();

    return 0;
}
