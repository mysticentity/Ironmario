// ironMarioTracker.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "ironMarioTracker.h"
#include <lua.hpp>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <imgui.h>

// Project64 memory addresses for tracking various stats
#define MARIO_X_ADDR 0x1A0340
#define MARIO_Y_ADDR 0x1A0344
#define MARIO_Z_ADDR 0x1A0348
#define STAR_COUNT_ADDR 0x1A0334
#define HEALTH_ADDR 0x1A0336
#define LEVEL_ID_ADDR 0x18FD78
#define SONG_ID_ADDR 0x19485E

// Handle to Project64 process
HANDLE pj64Handle = NULL;
bool show_overlay = true;
bool overlay_movable = true;
float overlay_opacity = 1.0f;
ImVec2 overlay_position = ImVec2(50, 50);

// Level and music data mappings
std::unordered_map<int, std::string> level_map = {
    {0, "Menu"}, {1, "Menu"}, {10, "Snowman's Land"}, {11, "Wet Dry World"},
    {12, "Jolly Roger Bay"}, {13, "Tiny Huge Island"}, {14, "Tick Tock Clock"},
    {15, "Rainbow Ride"}, {16, "Outside Castle"}, {17, "Bowser in the Dark World"},
    {18, "Vanish Cap Under the Moat"}, {19, "Bowser in the Fire Sea"},
    {20, "Secret Aquarium"}, {22, "Lethal Lava Land"}, {23, "Dire Dire Docks"},
    {24, "Whomp's Fortress"}, {26, "Garden"}, {27, "Peach's Slide"},
    {28, "Cavern of the Metal Cap"}, {29, "Tower of the Wing Cap"}, {30, "Bowser Fight 1"},
    {31, "Wing Mario Over the Rainbow"}, {36, "Tall Tall Mountain"}, {4, "Big Boo's Haunt"},
    {5, "Cool Cool Mountain"}, {6, "Castle"}, {7, "Hazy Maze Cave"},
    {8, "Shifting Sand Land"}, {9, "Bob-Omb Battlefield"}, {3626007, "Bowser in the Sky"}
};

std::unordered_map<int, std::string> music_map = {
    {12, "Super Mario 64 - Endless Stairs"}, {13, "Super Mario 64 - Merry Go Round"},
    {14, "Super Mario 64 - Title Screen"}, {15, "Super Mario 64 - Bob-omb Battlefield"},
    {16, "Super Mario 64 - Inside Castle"}, {17, "Super Mario 64 - Dire Dire Docks"},
    {18, "Super Mario 64 - Lethal Lava Land"}, {19, "Super Mario 64 - Snowman's Land"},
    {20, "Super Mario 64 - Cool Cool Mountain Slide"}, {21, "Super Mario 64 - Big Boo's Haunt"},
    {22, "Super Mario 64 - Piranha Plant Lullaby"}, {23, "Super Mario 64 - Hazy Maze Cave"},
    {24, "Super Mario 64 - Metal Cap"}, {25, "Super Mario 64 - Koopa Road"},
    {26, "Super Mario 64 - Bowser Battle"}, {27, "Super Mario 64 - File Select"},
    {28, "Super Mario 64 - Shell Power-up"}, {29, "Super Mario 64 - Rainbow Ride"}
};

// Attach to Project64 process
bool attach_to_pj64() {
    DWORD processID = 0;
    HWND hwnd = FindWindow(NULL, L"Project64");
    if (!hwnd) return false;
    GetWindowThreadProcessId(hwnd, &processID);
    pj64Handle = OpenProcess(PROCESS_VM_READ, FALSE, processID);
    return pj64Handle != NULL;
}

// Read memory from Project64
bool read_memory(DWORD address, void* buffer, SIZE_T size) {
    BOOL success = ReadProcessMemory(pj64Handle, reinterpret_cast<LPCVOID>(static_cast<SIZE_T>(address)), buffer, size, NULL);
    return success != 0; // Ensure function always returns true or false
}

// ImGui overlay for real-time tracking
void render_overlay() {
    if (!show_overlay) return;

    ImGui::SetNextWindowBgAlpha(overlay_opacity);
    ImGui::SetNextWindowPos(overlay_position, overlay_movable ? ImGuiCond_FirstUseEver : ImGuiCond_Always);

    ImGui::Begin("IronMario Tracker", &show_overlay, overlay_movable ? ImGuiWindowFlags_None : ImGuiWindowFlags_NoMove);

    float x, y, z;
    int stars, level_id, song_id;
    read_memory(MARIO_X_ADDR, &x, sizeof(float));
    read_memory(MARIO_Y_ADDR, &y, sizeof(float));
    read_memory(MARIO_Z_ADDR, &z, sizeof(float));
    read_memory(STAR_COUNT_ADDR, &stars, sizeof(int));
    read_memory(LEVEL_ID_ADDR, &level_id, sizeof(int));
    read_memory(SONG_ID_ADDR, &song_id, sizeof(int));

    std::string level_name = level_map.count(level_id) ? level_map[level_id] : "Unknown";
    std::string song_name = music_map.count(song_id) ? music_map[song_id] : "Unknown";

    ImGui::Text("Position: X=%.2f Y=%.2f Z=%.2f", x, y, z);
    ImGui::Text("Stars: %d", stars);
    ImGui::Text("Level: %s", level_name.c_str());
    ImGui::Text("Song: %s", song_name.c_str());

    ImGui::Separator();
    ImGui::Checkbox("Movable Overlay", &overlay_movable);
    ImGui::SliderFloat("Opacity", &overlay_opacity, 0.1f, 1.0f);

    ImGui::End();
}
int lua_get_level(lua_State* L);
int lua_get_song(lua_State* L);
// Function to register our module
extern "C" __declspec(dllexport) int luaopen_ironmario(lua_State* L) {
    static const luaL_Reg functions[] = {
        {"get_level", lua_get_level},
        {"get_song", lua_get_song},
        {NULL, NULL} // Sentinel value
    };

    lua_newtable(L);
    return 1;
    luaL_register(L, NULL, functions); // Return the library table
}

int lua_get_level(lua_State* L) {
    int level_id;
    if (read_memory(LEVEL_ID_ADDR, &level_id, sizeof(int))) {
        lua_pushinteger(L, level_id);
    }
    else {
        lua_pushnil(L);
    }
    return 1; // Returning 1 value (level ID)
}


int lua_get_song(lua_State* L) {
    int song_id;
    if (read_memory(SONG_ID_ADDR, &song_id, sizeof(int))) {
        lua_pushinteger(L, song_id);
    }
    else {
        lua_pushnil(L);
    }
    return 1; // Returning 1 value (song ID)
}
