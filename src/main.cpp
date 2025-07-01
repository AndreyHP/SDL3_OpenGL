#include "../include/glad/glad.h"
#include "../include/shader.h"
#include "../include/stb/stb_image.h"
#include "../include/glm/gtc/type_ptr.hpp"
#include "../include/camera.h"
#include "../include/model.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include "../include/imgui/imgui.h"
#include "../include/imgui/backends/imgui_impl_sdl3.h"
#include "../include/imgui/backends/imgui_impl_opengl3.h"



/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_GLContext glContext; // Use SDL_GLContext for OpenGL context

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

const int TARGET_FPS = 60;
const int FRAME_TIME = 1000 / TARGET_FPS; // Frame time in milliseconds

bool wireframe = false;
bool quit = false;
Shader Defaultshader;
int width, height, nrChannels;
int w, h;
float fov{45.0f};
float Zdistance{-5.0f};
float avgFPS = {0.0f};
float deltaTime = {0.0f};
bool  captureMouse{false};
bool  modelLoaded{false};
bool  showModel{false};

glm::mat4 model         = glm::mat4(1.0f);
glm::mat4 view          = glm::mat4(1.0f);
glm::mat4 projection    = glm::mat4(1.0f);

float yaw   = -90.0f;
float pitch =  0.0f;

void mouse_callback(double xpos, double ypos);
void scroll_callback(double xoffset, double yoffset);
void processEvents(SDL_Event *event);

int main(int argc, char *argv[]) {


    // Set application metadata
    SDL_SetAppMetadata("Example Renderer", "1.0", "com.example.renderer");

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Could not initialize SDL: %s", SDL_GetError());
        return 1;
    }

    const char* glsl_version = "#version 130";
    // Set OpenGL attributes
    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)) {
        SDL_Log("Could not set OpenGL attributes: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    
    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());


    // Create window
    window = SDL_CreateWindow("Example", SCR_WIDTH, SCR_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Could not create Window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }


    // Create OpenGL context
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        SDL_Log("Could not create OpenGL context: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

     // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init(glsl_version);


     // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
    io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }


     // Our state
    //bool show_demo_window = true;
    //bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SDL_Log("Failed to initialize GLAD");
        SDL_GL_DestroyContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    //load base model
    Model ourModel;

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);

    // Variables for FPS calculation
    Uint32 frameCount = 0;
    Uint32 startTime = SDL_GetTicks();
    Uint32 previousFrameTime = startTime; // Initialize with start time

    SDL_GetWindowSize(window, &w, &h);
    glViewport(0, 0, w, h);

    // Compile shaders
    Defaultshader.compile("./glsl/shader.vs", "./glsl/shader.fs");
    glEnable(GL_DEPTH_TEST);


    float scale = 1.0f;
    float rotateModel = 45.0f;
    float ZTranslate = 0.5f;
    float YTranslate = 0.0f;
    static char inputBuffer[256]; // Buffer for the text input
    // Main loop
    SDL_Event event;
    while (!quit) {
       Uint32 frameStart = SDL_GetTicks(); // Start time for the current frame
        processEvents(&event);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
            ImGui::SliderFloat("Angle", &rotateModel, -20.0f, 50.0f);
            ImGui::SliderFloat("Scale", &scale, 0.0f, 10.0f);
            ImGui::SliderFloat("YTranslate", &YTranslate, -20.0f, 20.0f);
            ImGui::SliderFloat("ZTranslate", &ZTranslate, -20.0f, 20.0f);
            ImGui::InputText("Model path", inputBuffer, sizeof(inputBuffer));
            // Create a button
            if (ImGui::Button("Submit")) {
                // Action to perform when the button is pressed
                // For example, print the input text to the console
                printf("Input: %s\n", inputBuffer);
                modelLoaded = true;
            }
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();

       
        // Render
        glClearColor(clear_color.x,clear_color.y,clear_color.z,clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        
        frameCount++;

        // Calculate FPS every second
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - startTime >= 1000) { // Update every 1 second
            avgFPS = frameCount / ((currentTime - startTime) / 1000.0f);
            //std::cout << "Average FPS: " << avgFPS << '\n';
            //std::cout << "Delta: " << deltaTime << '\n';
            frameCount = 0; // Reset frame count
            startTime = currentTime; // Reset timer
        }

        
        // Calculate delta time
        deltaTime = (currentTime - previousFrameTime) / 1000.0f; // Convert to seconds
        previousFrameTime = currentTime; // Update previous frame time

        Uint32 frameTime = SDL_GetTicks() - frameStart; // Time taken for the current frame
        if (frameTime < FRAME_TIME){
           //std::cout << FRAME_TIME - deltaTime;
            SDL_Delay(FRAME_TIME - frameTime);
        }

        // activate shader
        Defaultshader.use();
        // create transformations
        const double now = ((double)SDL_GetTicks()) / 1000.0;

        view = camera.GetViewMatrix();

        
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        Defaultshader.setMat4("projection", projection);
        Defaultshader.setMat4("view", view);

        float newAngle = rotateModel * now;
        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, YTranslate, ZTranslate));
        model = glm::rotate(model, glm::radians(newAngle), glm::vec3(0.0f, 0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(scale, scale, scale));
        Defaultshader.setMat4("model", model);

        if (modelLoaded){
            ourModel.textures_loaded.clear();
            ourModel.meshes.clear();

            ourModel.Load(inputBuffer);
            modelLoaded = false;
            showModel = true;
        }

        if (showModel){
            ourModel.Draw(Defaultshader);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
         
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }


        // Swap buffers
        SDL_GL_SwapWindow(window);

    }

    // Cleanup
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void processEvents(SDL_Event *event){
        
    const double delta = deltaTime;
    
    while (SDL_PollEvent(event)) {
           ImGui_ImplSDL3_ProcessEvent(event);
            
            if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_KEY_DOWN && event->key.scancode == SDL_SCANCODE_ESCAPE) {
                quit = true;
            }

            if (event->type == SDL_EVENT_WINDOW_RESIZED) {
                SDL_GetWindowSize(window, &w, &h);
                glViewport(0, 0, w, h); 
            }


            if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_RIGHT) {
                captureMouse = !captureMouse;
            }

            if (captureMouse){
                // Input
                SDL_SetWindowRelativeMouseMode(window, true);
                SDL_CaptureMouse(true);
                if (event->motion.type == SDL_EVENT_MOUSE_MOTION){
                    mouse_callback(event->motion.x, event->motion.y);
                }
            }else{
                // Input
                SDL_SetWindowRelativeMouseMode(window, false);
                SDL_CaptureMouse(false);
            }


            if (event->type == SDL_EVENT_KEY_DOWN && event->key.scancode == SDL_SCANCODE_W) {
                camera.ProcessKeyboard(FORWARD, delta);
            }
            if (event->type == SDL_EVENT_KEY_DOWN && event->key.scancode == SDL_SCANCODE_S) {
                camera.ProcessKeyboard(BACKWARD, delta);
            }
            if (event->type == SDL_EVENT_KEY_DOWN && event->key.scancode == SDL_SCANCODE_A) {
                camera.ProcessKeyboard(LEFT, delta);
            }
            if (event->type == SDL_EVENT_KEY_DOWN && event->key.scancode == SDL_SCANCODE_D) {
                camera.ProcessKeyboard(RIGHT, delta);
            }
            if (event->type == SDL_EVENT_KEY_DOWN && event->key.scancode == SDL_SCANCODE_F) {
                wireframe = !wireframe;
            }

        }

};

// sdl: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    // Get the current mouse state
    //float mouseX, mouseY;
    //SDL_GetMouseState(&mouseX, &mouseY);

    // Print the mouse position
    //printf("Mouse Position: (%f, %f)\n", mouseX, mouseY);

    // Warp the mouse back to the center of the window
    SDL_WarpMouseInWindow(window, SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f);

    // Calculate offsets
    float xoffset = xpos - (SCR_WIDTH / 2.0f);
    float yoffset = (SCR_HEIGHT / 2.0f) - ypos; // reversed since y-coordinates go from bottom to top

    // Update last positions
    lastX = SCR_WIDTH / 2.0f;
    lastY = SCR_HEIGHT / 2.0f;

    camera.ProcessMouseMovement(xoffset, yoffset);
}


// sdl: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

