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

#include "data_var_ui.h"

#include <map>
#include <vector>
#include <math.h>

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
static uint32_t send_bytes = 0;

void telemetry_native_recv(int max)
{
    while (serial_available() && max-- > 0)
    {
        uint8_t rd = 0;
        if (serial_read(&rd, 1) == 1)
        {
            telemetry_recv(rd);
        }
    }
}

static char port[64];
static int port_len;
static int baud = 400000;
static bool serialIsFile = false;
static bool serialLogToFile = false;

typedef struct TSampleData_t
{
    std::vector<float> values;
    float min;
    float max;
    float avg;
    int graphStart;

    static float PlotGetter(void *data, int idx)
    {
        auto sd = static_cast<const TSampleData_t *>(data);
        if (sd->values.empty())
            return 0.0f;

        return sd->values[(sd->graphStart + idx) % sd->values.size()];
    }
} TSampleData_t;

static std::map<int, TSampleData_t> samples;

// struct ImGuiPlotArrayGetterLoopData
// {
//     const float* Values;
//     int Stride;
//     int Size;

//     ImGuiPlotArrayGetterLoopData(const float* values, int stride, int size)
//     {
//         Values = values;
//         Stride = stride;
//         Size = size;
//     }
// };

// static float Plot_ArrayGetterLoop(void* data, int idx)
// {
//     ImGuiPlotArrayGetterLoopData* plot_data = (ImGuiPlotArrayGetterLoopData*)data;
//     const float v = *(const float*)(const void*)(((const unsigned char*)plot_data->Values + (size_t)idx * plot_data->Stride) % size);
//     return v;
// }

// void PlotLinesLooping(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride)
// {
//     ImGuiPlotArrayGetterLoopData data(values, stride, values_count);
//     ImGui::PlotEx(ImGuiPlotType_Lines, label, &Plot_ArrayGetterLoop, (void*)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
// }

void init_telemetry()
{
    serial_init();

    tdv_telemetry_str_table.v.u32 = 64 * 2 * tdv_telemetry_val_count.v.u32;
    tdv_telemetry_auto_register.v.b8 = true;
    telemetry_init();

    strcpy(port, "/dev/ttyACM0");
    port_len = strlen(port);
}

void applyFlatTheme()
{
    constexpr auto ColorFromBytes = [](uint8_t r, uint8_t g, uint8_t b)
    {
        return ImVec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
    };

    auto &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    ImVec4 accent(0.11f, 0.93f, 0.86f, 1.00f);
    ImVec4 accentHSV(0, 0, 0, 1.0f);

    ImGui::ColorConvertRGBtoHSV(
        accent.x, accent.y, accent.z,
        accentHSV.x, accentHSV.y, accentHSV.z);

    accentHSV.z *= 0.70f;
    ImGui::ColorConvertHSVtoRGB(
        accentHSV.x, accentHSV.y, accentHSV.z,
        accent.x, accent.y, accent.z);

    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.11f, 0.93f, 0.86f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.11f, 0.93f, 0.86f, 0.90f); // ImVec4(0.00f, 0.78f, 0.70f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = accent; // ImVec4(0.11f, 0.93f, 0.86f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.32f, 0.32f, 0.33f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.37f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.35f, 0.37f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.78f, 0.72f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.93f, 0.86f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.78f, 0.72f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.11f, 0.93f, 0.86f, 0.80f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.11f, 0.93f, 0.86f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.11f, 0.93f, 0.86f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.78f, 0.72f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.32f, 0.32f, 0.33f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.11f, 0.93f, 0.86f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.78f, 0.72f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.00f, 0.78f, 0.72f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 0.78f, 0.72f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.11f, 0.93f, 0.86f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 0.78f, 0.72f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.11f, 0.93f, 0.86f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.78f, 0.72f, 1.00f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;

    style.WindowPadding = ImVec2(8.00f, 8.00f);
    style.FramePadding = ImVec2(5.00f, 2.00f);
    style.CellPadding = ImVec2(6.00f, 6.00f);
    style.ItemSpacing = ImVec2(6.00f, 6.00f);
    style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
    style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
    style.IndentSpacing = 25;
    style.ScrollbarSize = 15;
    style.GrabMinSize = 10;
    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 1;
    style.TabBorderSize = 1;
    style.WindowRounding = 7;
    style.ChildRounding = 4;
    style.FrameRounding = 3;
    style.PopupRounding = 4;
    style.ScrollbarRounding = 9;
    style.GrabRounding = 3;
    style.LogSliderDeadzone = 4;
    style.TabRounding = 4;
}

void embraceTheDarkness()
{
    ImVec4 *colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
    colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
    colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    //   colors[ImGuiCol_DockingPreview]         = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    //   colors[ImGuiCol_DockingEmptyBg]         = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(8.00f, 8.00f);
    style.FramePadding = ImVec2(5.00f, 2.00f);
    style.CellPadding = ImVec2(6.00f, 6.00f);
    style.ItemSpacing = ImVec2(6.00f, 6.00f);
    style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
    style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
    style.IndentSpacing = 25;
    style.ScrollbarSize = 15;
    style.GrabMinSize = 10;
    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 1;
    style.TabBorderSize = 1;
    style.WindowRounding = 7;
    style.ChildRounding = 4;
    style.FrameRounding = 3;
    style.PopupRounding = 4;
    style.ScrollbarRounding = 9;
    style.GrabRounding = 3;
    style.LogSliderDeadzone = 4;
    style.TabRounding = 4;
}

void ShowDockSpace()
{
    static bool p_open = true;
    // static bool opt_fullscreen_persistant = true;
    // bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    // if (opt_fullscreen)
    {
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("###DockSpace", &p_open, window_flags);
    ImGui::PopStyleVar();
    // if (opt_fullscreen)
    ImGui::PopStyleVar(2);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Close"))
                ; // *p_open = false;
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // DockSpace
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    else
    {
        // ShowDockingDisabledMessage();
    }

    ImGui::End();
}

// Main code
int main(int, char **)
{
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
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
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    // applyFlatTheme();
    // embraceTheDarkness();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    io.Fonts->AddFontFromFileTTF("bin/Hack-Bold.ttf", 16.0f);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = false;
    bool show_another_window = false;
    // ImVec4 clear_color = ImVec4(0.094f, 0.094f, 0.094f, 1.000f);

    ImVec4 clear_color = ImVec4(0.235f, 0.235f, 0.235f, 1.000f);

    init_telemetry();

    // Main loop
    bool done = false;
    bool sampleValues = true;
    double lastSampleTime = 0;
    int samplesPerSecond = 60;
    int graphSamplesSecondsVisible = 5;
    float rangeAverage = 0.5f;
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

        double now = ImGui::GetTime();
        if (now - lastSampleTime >= 1.0f / samplesPerSecond)
        {
            sampleValues = true;
            lastSampleTime = now;
        }

        serial_stats(&recv_bytes, &send_bytes);

        telemetry_update();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ShowDockSpace();

        // // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        // if (show_demo_window)
        //     ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        // {
        //     static float f = 0.0f;
        //     static int counter = 0;

        //     ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

        //     ImGui::Text("This is some useful text.");          // Display some text (you can use a format strings too)
        //     ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
        //     ImGui::Checkbox("Another Window", &show_another_window);

        //     ImGui::SliderFloat("float", &f, 0.0f, 1.0f);             // Edit 1 float using a slider from 0.0f to 1.0f
        //     ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

        //     if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
        //         counter++;

        //     ImGui::SameLine();
        //     ImGui::Text("counter = %d", counter);

        //     if (ImGui::Button("Dark1"))
        //         applyFlatTheme();

        //     ImGui::SameLine();
        //     if (ImGui::Button("Dark2"))
        //         embraceTheDarkness();

        //     ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        //     ImGui::End();
        // }
        // create an ImGui window that covers the entire viewport, so that we can have a menu bar at the top of the applications

        if (ImGui::Begin("Serial")) // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        {
            bool isOpen = serial_is_open();

            ImGui::InputText("Port:", port, sizeof(port));
            ImGui::SameLine();
            if (isOpen)
                ImGui::BeginDisabled();
            ImGui::Checkbox("Is File", &serialIsFile);
            if (isOpen)
                ImGui::EndDisabled();

            ImGui::InputScalar("Baud:", ImGuiDataType_U32, &baud, NULL, NULL, "%u");

            if (serial_is_open())
            {
                if (ImGui::Button("Disconnect"))
                    serial_close();
            }
            else
            {
                if (ImGui::Button("Connect"))
                    serial_open(port, baud, serialIsFile, serialLogToFile);
            }

            if (isOpen)
                ImGui::BeginDisabled();
            ImGui::SameLine();
            ImGui::Checkbox("Log", &serialLogToFile);
            if (isOpen)
                ImGui::EndDisabled();

            ImGui::Text("rx: %u", recv_bytes);
            ImGui::SameLine();
            ImGui::Text("tx: %u", send_bytes);
        }
        ImGui::End();

        if (ImGui::Begin("Telemetry", &show_another_window, ImGuiWindowFlags_None))
        {
            // IMGUI_DEMO_MARKER("Examples/Simple layout");

            static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersInnerV
                                           // | ImGuiTableFlags_RowBg
                                           // | ImGuiTableFlags_NoBordersInBody
                                           | ImGuiTableFlags_SizingStretchProp;

            // ImGui::InputInt("Samples/Sec", &samplesPerSecond);
            // ImGui::SameLine();
            // ImGui::InputInt("Samples Visible Sec", &graphSamplesSecondsVisible);
            // ImGui::InputFloat("Graph Range Avg", &rangeAverage);

            if (ImGui::Button("Save To File"))
            {
                FILE *f = fopen("vars.txt", "w");
                if (f)
                {
                    telemetry_write_all_to_file(f, Tdm_all);
                    fclose(f);
                }
            }

            if (ImGui::BeginTable("Values", 5, flags))
            {
                int samplesVisible = (int)(samplesPerSecond * graphSamplesSecondsVisible);

                ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Graph");
                ImGui::TableSetupColumn("Description");
                // ImGui::TableSetupScrollFreeze(0, 1);
                // ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
                ImGui::TableHeadersRow();

                for (int id = 1; id < tdv_telemetry_val_count.v.u32; ++id)
                {
                    TDataVar_t *v = telemetry_get_var(id);
                    if (v == NULL)
                        continue;

                    ImGui::TableNextRow();

                    ImGui::PushID(id);
                    // Id
                    ImGui::TableNextColumn();
                    ImGui::Text("%u", id);

                    // Name
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", v->meta.name);

                    // Value
                    ImGui::TableNextColumn();

                    ShowDataVarEditor(v);

                    ImGui::TableNextColumn();

                    TSampleData_t &samplesData = samples[id];

                    if (!samplesData.values.empty() && v->meta.modsAllowed & Tdm_realtime)
                    {
                        ImGui::PlotLines(
                            "", &TSampleData_t::PlotGetter, (void *)&samplesData, samplesVisible - 1, 0, nullptr, samplesData.min, samplesData.max, ImVec2(0, 0));
                        // ImGui::SameLine();
                        // ImGui::Text("%.2f, %.2f, %.2f, %d"
                        //     , samplesData.min
                        //     , samplesData.max
                        //     , samplesData.avg
                        //     , samplesData.graphStart
                        //     );
                    }

                    // Description
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", v->meta.desc);

                    ImGui::PopID();
                    if (sampleValues)
                    {
                        float sv = telemetry_var_get_float(v);

                        if (samplesData.values.empty())
                        {
                            samplesData.graphStart = 0;
                            samplesData.min = sv;
                            samplesData.max = sv;
                            samplesData.avg = sv;
                            samplesData.values.resize(samplesVisible);
                        }
                        else
                        {
                            samplesData.avg += (sv - samplesData.avg) / samplesData.values.size();

                            if (sv > samplesData.max)
                                samplesData.max = sv;
                            else if (sv < samplesData.min)
                                samplesData.min = sv;
                        }

                        samplesData.min += (samplesData.avg - samplesData.min) / samplesData.values.size();
                        samplesData.max += (samplesData.avg - samplesData.max) / samplesData.values.size();

                        samplesData.values[samplesData.graphStart++] = sv;

                        samplesData.graphStart %= samplesData.values.size();
                    }
                }

                ImGui::EndTable();
            }
        }

        ImGui::End();

        sampleValues = false;
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