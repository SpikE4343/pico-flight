// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "lib_imgui.h"
#include "data_var_ui.h"
#ifdef __cplusplus
extern "C"
{
#endif

#include "telemetry.h"
#include "telemetry_native.h"

#ifdef __cplusplus
}
#endif




#include <map>
#include <vector>
#include <math.h>



// Main code
int ShowAllWindow()
{
        if (ImGui::Begin("All Vars", &show_another_window, ImGuiWindowFlags_MenuBar))
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


            static ImGuiTableFlags flags 
                = ImGuiTableFlags_BordersV 
                | ImGuiTableFlags_BordersOuterH 
                | ImGuiTableFlags_Resizable
                | ImGuiTableFlags_BordersInnerH
                | ImGuiTableFlags_BordersInnerV 
                // | ImGuiTableFlags_RowBg 
                // | ImGuiTableFlags_NoBordersInBody 
                | ImGuiTableFlags_SizingStretchProp;
            
            ImGui::InputInt("Samples/Sec", &samplesPerSecond);
            ImGui::SameLine();
            ImGui::InputInt("Samples Visible Sec", &graphSamplesSecondsVisible);
            ImGui::InputFloat("Graph Range Avg", &rangeAverage);

            if(ImGui::Button("Save To File"))
            {
                telemetry_write_all_to_file("vars.txt", Tdm_all);
            }

            if (ImGui::BeginTable("Values", 5, flags))
            {
                int samplesVisible = (int)(samplesPerSecond * graphSamplesSecondsVisible);

                ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Graph" );
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

                    TSampleData_t& samplesData = samples[id];
                    
                    ImGui::TableNextColumn();

                    if( !samplesData.values.empty() && v->meta.modsAllowed & Tdm_realtime)
                    {
                        ImGui::PlotLines( ""
                            , samplesData.values.data()
                            , samplesVisible-1
                            , (samplesData.graphStart-samplesVisible)
                            , nullptr
                            , samplesData.min
                            , samplesData.max
                            , ImVec2( 0, 0 )
                            , sizeof(float)
                        );
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
                    if(sampleValues)
                    {
                        if(samplesData.values.empty())
                        {
                            samplesData.graphStart = 0;
                            samplesData.min = sv;
                            samplesData.max = sv;
                            samplesData.avg = sv;
                            samplesData.values.resize(samplesVisible);
                        }
                        else
                        {
                            samplesData.avg += (sv - samplesData.avg) / samplesVisible;

                            if( sv > samplesData.max)
                                samplesData.max = sv;
                            else if( sv < samplesData.min)
                                samplesData.min = sv;
                        }


                        samplesData.min += (samplesData.avg - samplesData.min) / samplesVisible;
                        samplesData.max += (samplesData.avg - samplesData.max) / samplesVisible;

                        samplesData.values[samplesData.graphStart++] = sv;

                        samplesData.graphStart %= samplesData.values.size();
                    }
                }

                ImGui::EndTable();
            }

            
        }

        ImGui::End();

        sampleValues = false;
        
    }
}
