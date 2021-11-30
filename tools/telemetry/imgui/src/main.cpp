// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "lib_imgui.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "telemetry.h"
#include "telemetry_native.h"
#include "serial.h"

#ifdef __cplusplus
}
#endif

static telemetry_native_send_callback_t sendCallback;

bool telemetry_native_sending()
{
    return false;
}

void telemetry_native_init(telemetry_native_send_callback_t sendCB)
{
    sendCallback = sendCB;
    // s.sendCallback = sendCB;
    // dma_init(NULL);
}

void telemetry_native_send(int length, uint8_t *packets, int packetCount)
{
    serial_write(packets, length);
    if (sendCallback)
    {
        sendCallback(packetCount);
    }
    // dma_send(length, packets, packetCount);
}

static uint32_t recv_bytes = 0;
void telemetry_native_recv(int max)
{
    while (serial_available() && max-- > 0)
    {
        uint8_t rd = 0;
        if (serial_read(&rd, 1) == 1)
        {
            ++recv_bytes;
            telemetry_recv(rd);
        }
    }
}

static char port[64];
static int port_len;
static int baud = 115200;

void init_telemetry()
{
    serial_init();

    tdv_telemetry_str_table.v.u32 = 64 * 2 * tdv_telemetry_val_count.v.u32;
    tdv_telemetry_auto_register.v.b8 = true;
    telemetry_init();

   

    strcpy(port, "/dev/ttyACM0");
    port_len = strlen(port);
}

// Main code
int main(int, char **)
{
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char *glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char *glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("pico-flight-gui", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = false;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    init_telemetry();

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        telemetry_update();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");          // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);             // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        // if (show_another_window)
        // {
        //     ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        //     ImGui::Text("Hello from another window!");
        //     if (ImGui::Button("Close Me"))
        //         show_another_window = false;
        //     ImGui::End();
        // }

        ImGui::Begin("Serial", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::InputText("Port:", port, sizeof(port));
            ImGui::InputScalar("Baud:", ImGuiDataType_U32, &baud, NULL, NULL, "%u");
            
            if (serial_is_open())
            {
                if (ImGui::Button("Disconnect"))
                    serial_close();
            }
            else
            {
                if (ImGui::Button("Connect"))
                    serial_open(port, baud);
            }

            ImGui::Text("rx: %u", recv_bytes);

        ImGui::End();


        if (ImGui::Begin("Example: Simple layout", &show_another_window, ImGuiWindowFlags_MenuBar))
        {
            // IMGUI_DEMO_MARKER("Examples/Simple layout");
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Close"));// *p_open = false;
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

            if (ImGui::BeginTable("Values", 4, flags))
            {
                ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Description");
                // ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
                ImGui::TableHeadersRow();

                for (int id = 1; id < tdv_telemetry_val_count.v.u32; ++id)
                {
                    TDataVar_t *v = telemetry_get_var(id);
                    if (v == NULL)
                        continue;

                    ImGui::TableNextRow();
                    // Id
                    ImGui::TableNextColumn();
                    ImGui::Text("%u", id);

                    // Name
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", v->meta.name);

                    // Value
                    ImGui::TableNextColumn();    

                    bool canWrite = v->meta.modsAllowed & Tdm_write;

                    switch (v->meta.type)
                    {
                    case Tdt_u8:
                    case Tdt_u32:
                    case Tdt_u16:
                        // ImGui::InputInt("",(int*)&v->v.u32 );
                    // break;

                    case Tdt_i8:
                    case Tdt_i16:
                    case Tdt_i32:
                        if(canWrite)
                            ImGui::InputInt("",&v->v.i32 );
                        else
                            ImGui::Text("%d", v->v.i32);
                        break;

                    case Tdt_b8:
                        ImGui::Checkbox("", &v->v.b8);
                        break;
                    case Tdt_c8:
                        break;

                    case Tdt_f32:
                        if(canWrite)
                            ImGui::InputFloat("",&v->v.f32 );
                        else
                            ImGui::Text("%f", v->v.f32);
                        break;

                    default:
                        break;
                    }
                    // Description
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", v->meta.desc);
                }

                ImGui::EndTable();
            }
        }
        ImGui::End();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    serial_close();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
