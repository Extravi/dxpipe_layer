#pragma once
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "font.h"

inline void setImStyleTheme()
{
    ImGuiIO &io = ImGui::GetIO();

    // enable mouse cursor
    io.MouseDrawCursor = true;

    // set the font
    ImFontConfig font;
    font.FontDataOwnedByAtlas = false;

    // imgui theme
    ImGuiStyle &style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.WindowPadding = ImVec2(6.5f, 2.7f);
    style.WindowRounding = 10.0f; // even more rounded for a smooth look
    style.WindowBorderSize = 1.0f;
    style.WindowMinSize = ImVec2(20.0f, 32.0f);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_None;
    style.ChildRounding = 6.0f; // smoother children
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 8.0f; // smoother popups
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(20.0f, 3.5f);
    style.FrameRounding = 6.0f; // smoother frames
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(4.4f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.6f, 3.6f);
    style.IndentSpacing = 4.4f;
    style.ColumnsMinSpacing = 5.4f;

    // make scrollbar wider & grab handle bigger
    style.ScrollbarSize = 12.0f;     // was  8.8f
    style.ScrollbarRounding = 10.0f; // keep it nice and smooth
    style.GrabMinSize = 12.0f;       // was  9.4f
    style.GrabRounding = 6.0f;       // smoother grabs

    style.TabRounding = 6.0f; // smoother tabs
    style.TabBorderSize = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    // base text & backgrounds
    style.Colors[ImGuiCol_Text] = ImVec4(0.8784f, 0.8784f, 0.8784f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.6902f, 0.6902f, 0.6902f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0706f, 0.0706f, 0.0706f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0706f, 0.0706f, 0.0706f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1176f, 0.1176f, 0.1176f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.1725f, 0.1725f, 0.1725f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    // frames & hovers
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1176f, 0.1176f, 0.1176f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.08f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.12f);

    // title bar
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1176f, 0.1176f, 0.1176f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.12f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0706f, 0.0706f, 0.0706f, 0.5f);

    // menu & scrollbars
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1176f, 0.1176f, 0.1176f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0706f, 0.0706f, 0.0706f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1725f, 0.1725f, 0.1725f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.08f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.12f);

    // accent color
    ImVec4 accent = ImVec4(0.2431f, 0.3608f, 0.6196f, 1.0f);

    style.Colors[ImGuiCol_CheckMark] = accent;
    style.Colors[ImGuiCol_SliderGrab] = accent;
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.12f);

    style.Colors[ImGuiCol_Button] = ImVec4(0.1176f, 0.1176f, 0.1176f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.08f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.12f);

    // headers & separators
    style.Colors[ImGuiCol_Header] = ImVec4(0.1176f, 0.1176f, 0.1176f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.08f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.12f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.1725f, 0.1725f, 0.1725f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.08f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.12f);

    // grips & tabs
    style.Colors[ImGuiCol_ResizeGrip] = accent;
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.08f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.12f);

    style.Colors[ImGuiCol_Tab] = ImVec4(0.1176f, 0.1176f, 0.1176f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.15f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.20f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1176f, 0.1176f, 0.1176f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.12f);

    // plots & tables
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.1725f, 0.1725f, 0.1725f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.08f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.1725f, 0.1725f, 0.1725f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0000f, 1.0000f, 1.0000f, 0.08f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1176f, 0.1176f, 0.1176f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.1725f, 0.1725f, 0.1725f, 1.0f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.1725f, 0.1725f, 0.1725f, 0.5f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.1176f, 0.1176f, 0.1176f, 1.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.0706f, 0.0706f, 0.0706f, 1.0f);

    // selection & navigation
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.4f);
    style.Colors[ImGuiCol_DragDropTarget] = accent;
    style.Colors[ImGuiCol_NavHighlight] = accent;
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(accent.x, accent.y, accent.z, 0.7f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.0706f, 0.0706f, 0.0706f, 0.7f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0706f, 0.0706f, 0.0706f, 0.7f);

    io.Fonts->AddFontFromMemoryTTF((void *)rawData, sizeof(rawData), 17.0f, &font);
}