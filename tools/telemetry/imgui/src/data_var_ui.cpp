#include "data_var_ui.h"


bool InputUInt32(const char *label, uint32_t *v, int step, int step_fast, ImGuiInputTextFlags flags)
{
    // Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use InputText() to parse your own data, if you want to handle prefixes.
    const char *format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
    return ImGui::InputScalar(label, ImGuiDataType_U32, (void *)v, (void *)(step > 0 ? &step : NULL), (void *)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

bool ShowDataVarEditor(TDataVar_t *v)
{
    bool canWrite = v->meta.modsAllowed & Tdm_write;
    bool modified = false;
    float sv = 0.0f;
    int32_t i32 = v->v.i32;
    uint32_t u32 = v->v.u32;
    bool b8 = v->v.b8;
    float f32 = v->v.f32;

    switch (v->meta.type)
    {
    case Tdt_u8:
    case Tdt_u16:
    case Tdt_u32:
        if (canWrite)
            modified = InputUInt32("", &u32);
        else
            ImGui::Text("%u", v->v.u32);

        sv = (float)(v->v.u32 = u32);
        // ImGui::InputInt("",(int*)&v->v.u32 );
        break;

    case Tdt_i8:
    case Tdt_i16:
    case Tdt_i32:
        if (canWrite)
            modified = ImGui::InputInt("", &i32);
        else
            ImGui::Text("%d", v->v.i32);

        sv = (float)(v->v.i32 = i32);
        break;

    case Tdt_c8:
    case Tdt_b8:
        modified = ImGui::Checkbox("", &b8);
        sv = (float)(v->v.b8 = b8);
        break;

    case Tdt_f32:
        if (canWrite)
            modified = ImGui::InputFloat("", &f32);
        else
            ImGui::Text("%f", f32);
        sv = v->v.f32 = f32;
        break;

    default:
        break;
    }

    if (modified)
    {

        TDataModPacket_t packet = {
            .payload = {
                .mod = Tdm_write,
                .time = 0,
                .value = {
                    .id = v->id,
                    .type = v->meta.type,
                    .value = v->v}

            }};

        int sendLength = telemetry_write_data_frame(&packet);

        telemetry_native_send(sendLength, (uint8_t *)&packet, 1);
    }
}
