#include "../include/core.h"
#include "../include/appstate.h"
#include "../include/camera.h"
#include "../include/framebuffer.h"
#include "../include/glad/glad.h"
#include "../include/imgui/backends/imgui_impl_opengl3.h"
#include "../include/imgui/backends/imgui_impl_sdl3.h"
#include "../include/imgui/imgui.h"
#include "../include/model.h"
#include "../include/shader.h"
#include "../include/stb/stb_image.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <iostream>
#include <string>
#include <vector>

AppState appState;
SDL_Event event;

static const SDL_DialogFileFilter filters[] = {{"GLTF", "gltf;glb"}};

const int TARGET_FPS = 60;
const int FRAME_TIME = 1000 / TARGET_FPS; // Frame time in milliseconds

int width, height, nrChannels;
int w, h;
float fov{45.0f};
float Zdistance{-5.0f};
float avgFPS = {0.0f};
float deltaTime = {0.0f};
bool captureMouse{false};
bool showOutline{false};

glm::vec4 fogColor;


std::vector<std::string> items;
int selectedItem = -1; // To keep track of the selected item

glm::mat4 model = glm::mat4(1.0f);


ImGuiIO *gio;

float yaw = -90.0f;
float pitch = 0.0f;

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
float scale = 1.0f;
float rotateModel = 45.0f;
float ZTranslate = 0.5f;
float YTranslate = 0.0f;

// Variables for FPS calculation
Uint32 frameCount = 0;
Uint32 startTime = SDL_GetTicks();
Uint32 previousFrameTime = startTime;

bool postprecess{false};
Framebuffer framebuffer;
ScreenQuad screenquad;

bool quitApp{false};

namespace core {

AppState* GetAppState();
SDL_Event* GetEvent();
float GetDelta();
bool appDone();
void mouse_callback(double xpos, double ypos);
void scroll_callback(double xoffset, double yoffset);
void SetupImGUI(float mainScale, const char *glsl_version);
void file_dialog_callback(void *userdata, const char *const *filelist,
                          int filter);



void getViewportSize(int& width, int& height) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    width = viewport[2];
    height = viewport[3];
}

int on_init(void(*func)()) {
    // Set application metadata
    SDL_SetAppMetadata("3D Renderer", "1.0", "com.andrey.renderer");

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Could not initialize SDL: %s", SDL_GetError());
        return -1;
    }

    #if defined(GLSL_ES)
    const char *glsl_version = "#version 300 es";
    // Set OpenGL attributes
    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_ES)) {
        SDL_Log("Could not set OpenGL attributes: %s", SDL_GetError());
        SDL_Quit();
        return SDL_APP_FAILURE;
    }
    #else
    const char *glsl_version = "#version 410 core";
    // Set OpenGL attributes
    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE)) {
        SDL_Log("Could not set OpenGL attributes: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    #endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    // Create window
    appState.window =
        SDL_CreateWindow("3d Render", appState.SCR_WIDTH, appState.SCR_HEIGHT,
                        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!appState.window) {
        SDL_Log("Could not create Window: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // Create OpenGL context
    appState.glContext = SDL_GL_CreateContext(appState.window);
    if (!appState.glContext) {
        SDL_Log("Could not create OpenGL context: %s", SDL_GetError());
        SDL_DestroyWindow(appState.window);
        SDL_Quit();
        return -1;
    }

    SDL_GL_MakeCurrent(appState.window, appState.glContext);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    SDL_SetWindowPosition(appState.window, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED);

    SetupImGUI(main_scale, glsl_version);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SDL_Log("Failed to initialize GLAD");
        SDL_GL_DestroyContext(appState.glContext);
        SDL_DestroyWindow(appState.window);
        SDL_Quit();
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading
    // model).
    // stbi_set_flip_vertically_on_load(true);

    SDL_GetWindowSize(appState.window, &w, &h);
    glViewport(0, 0, w, h);

    glEnable(GL_DEPTH_TEST);

    #if defined(GLSL_ES)
    // Compile shaders
    appState.defaultShader.compile("./glsl/web/Vert_psx.glsl",
                                    "./glsl/web/Frag_shader.glsl");

    appState.singleColorShader.compile(
        "./glsl/web/Vert_stencil_testing.glsl",
        "./glsl/web/Frag_stencil_single_color.glsl");
    #else
    // Compile shaders
    appState.defaultShader.compile("./glsl/desktop/Vert_psx.glsl",
                                    "./glsl/desktop/Frag_shader.glsl");

    appState.singleColorShader.compile(
        "./glsl/desktop/Vert_stencil_testing.glsl",
        "./glsl/desktop/Frag_stencil_single_color.glsl");
    #endif
    // activate shader
    appState.defaultShader.use();
    appState.defaultShader.setInt("texture_diffuse1", 0);

    #if defined(GLSL_ES)
    // Initialize framebuffer and screen quad
    framebuffer.Create(w, h, false);
    screenquad.Create("./glsl/web/Vert_postprocess.glsl",
                        "./glsl/web/Frag_postprocess.glsl");
    #else
    // Initialize framebuffer and screen quad
    framebuffer.Create(w, h, false);
    screenquad.Create("./glsl/desktop/Vert_postprocess.glsl",
                        "./glsl/desktop/Frag_postprocess.glsl");
    #endif



#ifdef __EMSCRIPTEN__
    //  list of models
    items = {
    "./assets/cube/cube.gltf",
    "./assets/ironman/scene.gltf",
    "./assets/KokiriForest/scene.gltf",
        };
#endif

    func();

    return 0;
}

int on_event(void(*func)()) {

    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_QUIT) {
        return quitApp = true; /* end the program, reporting success to the OS. */
        }

        if (event.type == SDL_EVENT_QUIT ||
            event.type == SDL_EVENT_KEY_DOWN &&
                event.key.scancode == SDL_SCANCODE_ESCAPE) {
        return quitApp = true;
        }

        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
        SDL_GetWindowSize(appState.window, &w, &h);
        //int vw, vh;
        //getViewportSize(vw, vh);
        glViewport(0, 0, w, h);
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            event.button.button == SDL_BUTTON_RIGHT) {
        captureMouse = !captureMouse;
        }

        if (captureMouse) {
        // Input
        SDL_SetWindowRelativeMouseMode(appState.window, true);
        SDL_CaptureMouse(true);
        if (event.motion.type == SDL_EVENT_MOUSE_MOTION) {
            mouse_callback(event.motion.x, event.motion.y);
        }
        } else {
        // Input
        SDL_SetWindowRelativeMouseMode(appState.window, false);
        SDL_CaptureMouse(false);
        }

        func();

    }

    return 0;
}

int on_update(void(*func)()) {

    // create transformations
    Uint32 frameStart = SDL_GetTicks(); // Start time for the current frame
    const double now = ((double)frameStart) / 1000.0;
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and
                                    // append into it.

    ImGui::ColorEdit3(
        "clear color",
        (float *)&clear_color); // Edit 3 floats representing a color
    ImGui::SliderFloat("Angle", &rotateModel, 0.0f, 50.0f);
    ImGui::SliderFloat("Scale", &scale, 0.0f, 10.0f);
    ImGui::SliderFloat("YTranslate", &YTranslate, -20.0f, 20.0f);
    ImGui::SliderFloat("ZTranslate", &ZTranslate, -20.0f, 20.0f);

    // Models list
    ImGui::Text("Models:");
    if (ImGui::ListBox(
            "Items", &selectedItem,
            [](void *data, int idx, const char **out_text) {
                *out_text =
                    static_cast<std::vector<std::string> *>(data)->at(idx).c_str();
                return true;
            },
            static_cast<void *>(&items), items.size())) {
        // Update inputBuffer with the selected item
        if (selectedItem >= 0 && selectedItem < items.size()) {
        strncpy(appState.inputBuffer, items[selectedItem].c_str(),
                sizeof(appState.inputBuffer) - 1);
        appState.inputBuffer[sizeof(appState.inputBuffer) - 1] = '\0'; // Ensure null-termination
        }
    }
    if (ImGui::Button("Open")) {
        #ifdef __EMSCRIPTEN__
        //SDL_ShowOpenFileDialog(file_dialog_callback, NULL, appState.window, filters,1, NULL, false);
        #else
        SDL_ShowOpenFileDialog(file_dialog_callback, NULL, appState.window, filters,1, NULL, false);
        #endif
    }
    // Create a button
    if (ImGui::Button("Load")) {
        // Action to perform when the button is pressed
        // For example, print the input text to the console
        std::cout << "Imput: " << appState.inputBuffer << std::endl;
        appState.modelLoaded = true;
    }
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / gio->Framerate, gio->Framerate);
    ImGui::End();

    if (postprecess) {
        // Render to framebuffer
        framebuffer.Bind();
    }
    // Render
    fogColor.r = clear_color.x;
    fogColor.g = clear_color.y;
    fogColor.b = clear_color.z;
    fogColor.a = clear_color.w;
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    appState.defaultShader.setVec4("fogColor", fogColor.r, fogColor.g, fogColor.b,
                                    fogColor.a);

    #if defined(GLSL_ES)
    // if (appState.wireframe) {
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // } else {
    //   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //}
    #else
    if (appState.wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    #endif

    frameCount++;

    // Calculate FPS every second
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - startTime >= 1000) { // Update every 1 second
        avgFPS = frameCount / ((currentTime - startTime) / 1000.0f);
        // std::cout << "Average FPS: " << avgFPS << '\n';
        // std::cout << "Delta: " << deltaTime << '\n';
        frameCount = 0;          // Reset frame count
        startTime = currentTime; // Reset timer
    }

    // Calculate delta time
    deltaTime = (currentTime - previousFrameTime) / 1000.0f; // Convert to seconds
    previousFrameTime = currentTime; // Update previous frame time

    Uint32 frameTime =
        SDL_GetTicks() - frameStart; // Time taken for the current frame
    if (frameTime < FRAME_TIME) {
        // std::cout << FRAME_TIME - deltaTime;
        SDL_Delay(FRAME_TIME - frameTime);
    }



    func();


    if (postprecess) {
        // Render framebuffer to screen with post-processing
        framebuffer.Unbind(w, h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        screenquad.Draw(framebuffer.textureColorBuffer);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers
    SDL_GL_SwapWindow(appState.window);

    return 0;
}

void on_quit() {
    framebuffer.Delete();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(appState.glContext);
    SDL_DestroyWindow(appState.window);
    SDL_Quit();
}

// sdl: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    // Get the current mouse state
    // float mouseX, mouseY;
    // SDL_GetMouseState(&mouseX, &mouseY);

    // Print the mouse position
    // printf("Mouse Position: (%f, %f)\n", mouseX, mouseY);

    // Warp the mouse back to the center of the window
    SDL_WarpMouseInWindow(appState.window, appState.SCR_WIDTH / 2.0f,
                            appState.SCR_HEIGHT / 2.0f);

    // Calculate offsets
    float xoffset = xpos - (appState.SCR_WIDTH / 2.0f);
    float yoffset = (appState.SCR_HEIGHT / 2.0f) -
                    ypos; // reversed since y-coordinates go from bottom to top

    // Update last positions
    appState.lastX = appState.SCR_WIDTH / 2.0f;
    appState.lastY = appState.SCR_HEIGHT / 2.0f;

    appState.camera.ProcessMouseMovement(xoffset, yoffset);
}

// sdl: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

// Function to access ImGuiIO globally
void SetupImGUI(float mainScale, const char *glsl_version) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad;            // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(appState.window, appState.glContext);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup scaling
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(
        mainScale); // Bake a fixed style scale. (until we have a solution for
                    // dynamic style scaling, changing this requires resetting
                    // Style + calling this again)
    style.FontScaleDpi =
        mainScale; // Set initial font scale. (using io.ConfigDpiScaleFonts=true
                    // makes this unnecessary. We leave both here for documentation
                    // purpose)
    io.ConfigDpiScaleFonts =
        true; // [Experimental] Automatically overwrite style.FontScaleDpi in
                // Begin() when Monitor DPI changes. This will scale fonts but _NOT_
                // scale sizes/padding for now.
    io.ConfigDpiScaleViewports =
        true; // [Experimental] Scale Dear ImGui and Platform Windows when Monitor
                // DPI changes.

    gio = &io;
}

void file_dialog_callback(void *userdata, const char *const *filelist,
                          int filter) {
    if (!filelist) {
        SDL_Log("An error occured: %s", SDL_GetError());
        return;
    } else if (!*filelist) {
        SDL_Log("The user did not select any file.");
        SDL_Log("Most likely, the dialog was canceled.");
        return;
    }

    string s = "dasd";
    s = *filelist;

    // Ensure the string fits in the char array
    if (s.size() < sizeof(appState.inputBuffer)) {
        // Copy the string to the char array
        strncpy(appState.inputBuffer, s.c_str(), sizeof(appState.inputBuffer) - 1);
        // Ensure null termination
        appState.inputBuffer[sizeof(appState.inputBuffer) - 1] = '\0';
    } else {
        std::cerr << "String is too long to fit in char array." << std::endl;
    }


    items.push_back(s);

    // std::cout <<inputBuffer << std::endl;
}

bool appDone() {
    bool v{false};
    if (quitApp) {
        v = !v;
    }
    return v;
    };

SDL_Event* GetEvent(){
    return &event;
};

float GetDelta(){
    return deltaTime;
};

AppState* GetAppState(){
    return &appState;
};

}; // namespace core
