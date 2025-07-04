#include <SDL3/SDL.h>
#include "./glm/glm.hpp"
#include "shader.h"
#include "camera.h"
#include "model.h"
#include "../include/imgui/imgui.h"

struct AppState {
    // SDL & OpenGL
    SDL_Window *window = nullptr;
    SDL_GLContext glContext = nullptr;

    // Window size
    int SCR_WIDTH = 1280;
    int SCR_HEIGHT = 720;

    // Camera
    Camera camera{glm::vec3(0.0f, 0.0f, 5.0f)};
    float lastX = SCR_WIDTH / 2.0f;
    float lastY = SCR_HEIGHT / 2.0f;

    // Shaders
    Shader defaultShader;
    Shader mainShader;
    Shader singleColorShader;

    // Model
    Model model;
    bool modelLoaded = false;
    bool showModel = false;
    bool showOutline = false;

    // UI state
    ImGuiIO* imguiIO = nullptr;
    ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    char inputBuffer[256] = {};

    // Rendering state
    float scale = 1.0f;
    float rotateModel = 45.0f;
    float yTranslate = 0.0f;
    float zTranslate = 0.5f;
    bool wireframe = false;
    bool captureMouse = false;

    // Timing
    float fov = 45.0f;
    float avgFPS = 0.0f;
    float deltaTime = 0.0f;
    Uint32 frameCount = 0;
    Uint32 startTime = 0;
    Uint32 previousFrameTime = 0;
};
