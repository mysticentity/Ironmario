#include <duktape.h>
#include <Ultralight/Ultralight.h>
#include <AppCore/AppCore.h>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <windows.h>

using namespace ultralight;

// Memory Addresses
#define MARIO_BASE 0x1A0340
#define HUD_BASE 0x1A0330
#define CURRENT_LEVEL_ID 0x18FD78
#define CURRENT_SONG_ID 0x19485E
#define ATTEMPT_COUNT_ADDR 0x1CDF80

// Global Variables
HANDLE gameProcess = nullptr;
DWORD processID = 0;
void* gameBaseAddress = nullptr;

// Ultralight GUI Handler
class GUI : public ViewListener, public LoadListener {
public:
    RefPtr<Window> window;
    RefPtr<Overlay> overlay;
    RefPtr<App> app;

    GUI() {
        app = App::Create();
        window = Window::Create(app->main_monitor(), 800, 400, false, kWindowFlags_Titled);
        overlay = Overlay::Create(window, 800, 400, 1.0);
        LoadDefaultUI();
    }

    void LoadDefaultUI() {
        overlay->view()->LoadHTML(R"(
        <html>
        <head>
            <style>
                body { font-family: Arial, sans-serif; color: white; background: black; }
                .container { padding: 10px; }
                .header { font-size: 24px; font-weight: bold; text-align: center; }
                .stats { font-size: 18px; margin-top: 10px; }
                .warp-map { margin-top: 20px; }
                .warp-map h2 { font-size: 20px; color: yellow; }
                .warp-map ul { list-style: none; padding: 0; }
                .warp-map li { font-size: 16px; }
            </style>
        </head>
        <body>
            <div class="container">
                <div class="header">IronMario Tracker</div>
                <div class="stats">Waiting for game data...</div>
            </div>
        </body>
        </html>
        )");
    }

    void UpdateUI(const std::string& html) {
        overlay->view()->LoadHTML(html);
    }

    void Run() {
        app->Run();
    }
};

// Global GUI instance
GUI gui;

// Game state variables
int attempts = 0;
int stars = 0;
int pb_stars = 0;
std::string current_stage = "Unknown";
std::string current_stage_abbr = "???";
std::string current_song = "Unknown";
std::string run_time = "00:00:00";
std::unordered_map<std::string, std::string> warp_map;

// Memory Reading Function
bool ReadMemory(uintptr_t address, void* buffer, SIZE_T size) {
    return ReadProcessMemory(gameProcess, (LPCVOID)(reinterpret_cast<uintptr_t>(gameBaseAddress) + address), buffer, size, NULL);
}

// Get Level Name (Mapping from Lua)
std::string GetLevelName(int levelID) {
    static std::unordered_map<int, std::string> levelMap = {
        {10, "Snowman's Land"}, {11, "Wet Dry World"}, {12, "Jolly Roger Bay"},
        {13, "Tiny Huge Island"}, {14, "Tick Tock Clock"}, {15, "Rainbow Ride"},
        {16, "Outside Castle"}, {17, "Bowser in the Dark World"}
    };
    return levelMap.count(levelID) ? levelMap[levelID] : "Unknown";
}

// Get Song Name (Mapping from Lua)
std::string GetSongName(int songID) {
    static std::unordered_map<int, std::string> songMap = {
        {12, "Endless Stairs"}, {13, "Merry Go Round"}, {14, "Title Screen"},
        {15, "Bob-omb Battlefield"}, {16, "Inside Castle"}, {17, "Dire Dire Docks"}
    };
    return songMap.count(songID) ? songMap[songID] : "Unknown";
}

// Function to update game state dynamically
void UpdateGameState() {
    while (true) {
        int levelID = 0, songID = 0, starCount = 0;
        ReadMemory(CURRENT_LEVEL_ID, &levelID, sizeof(levelID));
        ReadMemory(CURRENT_SONG_ID, &songID, sizeof(songID));
        ReadMemory(HUD_BASE + 0x4, &starCount, sizeof(starCount));
        ReadMemory(ATTEMPT_COUNT_ADDR, &attempts, sizeof(attempts));

        current_stage = GetLevelName(levelID);
        current_stage_abbr = std::to_string(levelID);
        current_song = GetSongName(songID);
        stars = starCount;

        std::string html = R"(
        <html>
        <head>
            <style>
                body { font-family: Arial, sans-serif; color: white; background: black; }
                .container { padding: 10px; }
                .header { font-size: 24px; font-weight: bold; text-align: center; }
                .stats { font-size: 18px; margin-top: 10px; }
                .warp-map { margin-top: 20px; }
                .warp-map h2 { font-size: 20px; color: yellow; }
                .warp-map ul { list-style: none; padding: 0; }
                .warp-map li { font-size: 16px; }
            </style>
        </head>
        <body>
            <div class="container">
                <div class="header">IronMario Tracker</div>
                <div class="stats">
                    <p><b>Attempt:</b> )" + std::to_string(attempts) + R"(</p>
                    <p><b>Stars:</b> )" + std::to_string(stars) + " / PB: " + std::to_string(pb_stars) + R"(</p>
                    <p><b>Stage:</b> )" + current_stage + " (" + current_stage_abbr + R"()</p>
                    <p><b>Current Song:</b> )" + current_song + R"(</p>
                    <p><b>Run Time:</b> )" + run_time + R"(</p>
                </div>
            </div>
        </body>
        </html>
        )";

        gui.UpdateUI(html);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Initialize GUI and memory reading
static duk_ret_t init_gui(duk_context* ctx) {
    std::cout << "Initializing GUI..." << std::endl;
    std::thread guiThread([] { gui.Run(); });
    guiThread.detach();

    HWND gameWindow = FindWindowA(NULL, "Project64");
    if (gameWindow) {
        GetWindowThreadProcessId(gameWindow, &processID);
        gameProcess = OpenProcess(PROCESS_VM_READ, FALSE, processID);
        if (!gameProcess) {
            std::cerr << "Failed to attach to game process." << std::endl;
        }
    }

    std::thread memoryThread(UpdateGameState);
    memoryThread.detach();

    return 0;
}

// Register DLL in Duktape
extern "C" __declspec(dllexport) void dukopen_IronMarioTracker(duk_context* ctx) {
    duk_push_global_object(ctx);
    duk_push_object(ctx);
    duk_push_c_function(ctx, init_gui, 0);
    duk_put_prop_string(ctx, -2, "init_gui");
    duk_put_prop_string(ctx, -2, "IronMarioTracker");
}
