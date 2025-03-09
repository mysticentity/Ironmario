#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <ctime>
#include <json/json.h> // JSON handling library
#include <windows.h>
#include <unordered_map>
#include <tlhelp32.h>
#ifdef _WIN32


// Memory Addresses
struct MEM {
    static constexpr uintptr_t MARIO_BASE = 0x8033B170;
    static constexpr uintptr_t HUD_BASE = 0xE00103E4;
    static constexpr uintptr_t CURRENT_LEVEL_ID = 0xDFFFFD7A;
    static constexpr uintptr_t CURRENT_SEED = 0x1Cdf80;
    static constexpr uintptr_t DELAYED_WARP_OP = 0x1a031c;
    static constexpr uintptr_t INTENDED_LEVEL_ID = 0x19f0cc;
    static constexpr uintptr_t CURRENT_SONG_ID = 0xDFF0F438;
    
};

struct CONFIG {
    struct MEM_MARIO {
        static constexpr uintptr_t INPUT = MEM::MARIO_BASE + 0x2;
        static constexpr uintptr_t ACTION = MEM::MARIO_BASE + 0xC;
        static constexpr uintptr_t POS = MEM::MARIO_BASE + 0x3C;
        static constexpr uintptr_t HURT_COUNTER = MEM::MARIO_BASE + 0xB2;
    };

    struct MEM_HUD {
        static constexpr uintptr_t STARS = MEM::HUD_BASE + 0x4;
        static constexpr uintptr_t HEALTH = MEM::HUD_BASE + 0x6;
    };

    struct MUSIC_DATA {
        static const std::unordered_map<int, std::string> SONG_MAP;
    };
    struct LEVEL_DATA {
        static const std::unordered_map<int, std::string> LEVEL_MAP;
    };
};


const std::unordered_map<int, std::string> CONFIG::MUSIC_DATA::SONG_MAP = {
    {12, "Super Mario 64 - Endless Staircase"},
    {13, "Super Mario 64 - Merry-Go-Round"},
    {14, "Super Mario 64 - Title Theme"},
    {15, "Super Mario 64 - Bob-omb Battlefield"},
    {16, "Super Mario 64 - Inside the Castle Walls"},
    {17, "Super Mario 64 - Dire, Dire Docks"},
    {18, "Super Mario 64 - Lethal Lava Land"},
    {19, "Super Mario 64 - Koopa's Theme"},
    {20, "Super Mario 64 - Snow Mountain"},
    {21, "Super Mario 64 - Slider"},
    {22, "Super Mario 64 - Haunted House"},
    {23, "Super Mario 64 - Piranha Plant's Lullaby"},
    {24, "Super Mario 64 - Cave Dungeon"},
    {25, "Super Mario 64 - Powerful Mario"},
    {26, "Super Mario 64 - Metallic Mario"},
    {27, "Super Mario 64 - Koopa's Road"},
    {28, "Race Fanfare"},
    {29, "Super Mario 64 - Stage Boss"},
    {30, "Super Mario 64 - Ultimate Koopa"},
    {31, "Super Mario 64 - File Select"},
    {32, "Super Mario 64 - Powerful Mario"},
{33, "Super Mario 64 - Title Theme"},
{34, "Bomberman 64 - Green Garden"},
{35, "Bomberman 64 - Blue Resort"},
{36, "Bomberman Hero - Redial"},
{37, "Wii Shop Channel"},
{38, "Chrono Trigger - Delightful Spekkio"},
{39, "Castlevania: Order of Ecclesia - A Prologue"},
{40, "Diddy Kong Racing - Darkmoon Caverns"},
{41, "Diddy Kong Racing - Frosty Village"},
{42, "Diddy Kong Racing - Spacedust Alley, Star City"},
{43, "Donkey Kong Country - Aquatic Ambience"},
{44, "Donkey Kong Country 2 - Forest Interlude"},
{45, "Donkey Kong Country 2 - Stickerbrush Symphony"},
{46, "Diddy Kong Racing - Greenwood Village"},
{47, "Donkey Kong Country 2 - In a Snow-Bound Land"},
{48, "Earthbound - Home Sweet Home"},
{49, "Earthbound - Onett"},
{50, "TLoZ: Ocarina of Time - Gerudo Valley"},
{51, "Pokemon Shuffle - Stage (Hard)"},
{52, "Banjo-Kazooie - Gruntilda's Lair"},
{53, "Kirby: Nightmare in Dream Land - Butter Building"},
{54, "Kirby 64: The Crystal Shards - Shiver Star"},
{55, "Kirby's Adventure - Yogurt Yard"},
{56, "Kirby Super Star - Mine Cart Riding"},
{57, "TLoZ: Majora's Mask - Clock Town Day 1"},
{58, "Mario & Luigi: Partners in Time - Thwomp Caverns"},
{59, "Mario Kart 8 - Rainbow Road"},
{60, "Mario Kart 64 - Koopa Beach"},
{61, "Mario Kart Wii - Maple Treeway"},
{62, "Mega Man 3 - Spark Man Stage"},
{63, "Mega Man Battle Network 5 - Hero Theme"},
{64, "Mario Kart 64 - Moo Moo Farm, Yoshi Valley"},
{65, "New Super Mario Bros. - Athletic Theme"},
{66, "New Super Mario Bros. - Desert Theme"},
{67, "New Super Mario Bros. U - Overworld"},
{68, "New Super Mario Bros. Wii - Forest"},
{69, "TLoZ: Ocarina of Time - Lost Woods"},
{70, "Pilotwings - Light Plane"},
{71, "Pokemon Diamond and Pearl - Eterna Forest"},
{72, "Pokemon HeartGold and SoulSilver - Lavender Town"},
{73, "Mario Party - Mario's Rainbow Castle"},
{74, "Bomberman 64 - Red Mountain"},
{75, "Deltarune - Rude Buster"},
{76, "Super Mario 3D World - Overworld"},
{77, "Super Mario Sunshine - Sky and Sea"},
{78, "Snowboard Kids - Big Snowman"},
{79, "Sonic Adventure - Emerald Coast"},
{80, "Sonic the Hedgehog - Green Hill Zone"},
{81, "Super Castlevania 4 - The Submerged City"},
{82, "Super Mario Land - Birabuto Kingdom"},
{83, "Super Mario RPG - Beware the Forest's Mushrooms"},
{84, "Super Mario Sunshine - Delfino Plaza"},
{85, "Super Mario Sunshine - Gelato Beach"},
{86, "Yoshi's Island - Crystal Caves"},
{87, "TLoZ: Ocarina of Time - Water Temple"},
{88, "Wave Race 64 - Sunny Beach"},
{89, "Whomp's Floating Habitat (Original by MariosHub)"},
{90, "TLoZ: Ocarina of Time - Kokiri Forest"},
{91, "TLoZ: Ocarina of Time - Zora's Domain"},
{92, "TLoZ: Ocarina of Time - Kakariko Village"},
{93, "A Morning Jog (Original by TheGael95)"},
{94, "TLoZ: The Wind Waker - Outset Island"},
{95, "Super Paper Mario - Flipside"},
{96, "Super Mario Galaxy - Ghostly Galaxy"},
{97, "Super Mario RPG - Nimbus Land"},
{98, "Super Mario Galaxy - Battlerock Galaxy"},
{99, "Sonic Adventure - Windy Hill"},
{100, "Super Paper Mario - The Overthere Stair"},
{101, "Super Mario Sunshine - Secret Course"},
{102, "Super Mario Sunshine - Bianco Hills"},
{103, "Super Paper Mario - Lineland Road"},
{104, "Paper Mario: The Thousand-Year Door - X-Naut Fortress"},
{105, "Mario and Luigi: Bowser's Inside Story - Bumpsy Plains"},
{106, "Super Mario World - Athletic Theme"},
{107, "TLoZ: Skyward Sword - Skyloft"},
{108, "Super Mario World - Castle"},
{109, "Super Mario Galaxy - Comet Observatory"},
{110, "Banjo-Kazooie - Freezeezy Peak"},
{111, "Mario Kart DS - Waluigi Pinball"},
{112, "Kirby 64: The Crystal Shards - Factory Inspection"},
{113, "Donkey Kong 64 - Creepy Castle"},
{114, "Paper Mario 64 - Forever Forest"},
{115, "Super Mario Bros. - Bowser's Theme (Remix)"},
{116, "TLoZ: Twilight Princess - Gerudo Desert"},
{117, "Yoshi's Island - Overworld"},
{118, "Mario and Luigi: Partners in Time - Gritzy Desert"},
{119, "Donkey Kong 64 - Angry Aztec"},
{120, "Mario and Luigi: Partners in Time - Yoshi's Village"},
{121, "Touhou 10: Mountain of Faith - Youkai Mountain"},
{122, "Mario and Luigi: Bowser's Inside Story - Deep Castle"},
{123, "Paper Mario: The Thousand-Year Door - Petal Meadows"},
{124, "Mario Party - Yoshi's Tropical Island"},
{125, "Super Mario 3D World - Piranha Creek"},
{126, "Final Fantasy VII - Forested Temple"},
{127, "Paper Mario 64 - Dry Dry Desert"},
{128, "Rayman - Band Land"},
{129, "Donkey Kong 64 - Hideout Helm"},
{130, "Donkey Kong 64 - Frantic Factory"},
{131, "Super Paper Mario - Sammer's Kingdom"},
{132, "Super Mario Galaxy - Purple Comet"},
{133, "TLoZ: Majora's Mask - Stone Tower Temple"},
{134, "Banjo-Kazooie - Bubblegloop Swamp"},
{135, "Banjo-Kazooie - Gobi's Valley"},
{136, "Bomberman 64 - Black Fortress"},
{137, "Donkey Kong 64 - Fungi Forest"},
{138, "Paper Mario: The Thousand-Year Door - Riddle Tower"},
{139, "Paper Mario: The Thousand-Year Door - Rogueport Sewers"},
{140, "Super Mario Galaxy 2 - Honeybloom Galaxy"},
{141, "Pokemon Mystery Dungeon - Sky Tower"},
{142, "Super Mario Bros. 3 - Overworld"},
{143, "Super Mario RPG - Mario's Pad"},
{144, "Super Mario RPG - Sunken Ship"},
{145, "Super Mario Galaxy - Buoy Base Galaxy"},
{146, "Donkey Kong 64 - Crystal Caves"},
{147, "Super Paper Mario - Floro Caverns"},
{148, "Yoshi's Story - Title Theme"},
{149, "TLoZ: Twilight Princess - Lake Hylia"},
{150, "Mario Kart 64 - Frappe Snowland"},
{151, "Donkey Kong 64 - Gloomy Galleon"},
{152, "Mario Kart 64 - Bowser's Castle"},
{153, "Mario Kart 64 - Rainbow Road"},
{154, "Banjo-Kazooie - Mumbo's Mountain"},
{155, "Donkey Kong Country 2 - Jib Jig"},
{156, "Donkey Kong Country 2 - Welcome to Crocodile Isle"},
{157, "TLoZ: The Wind Waker - Dragon Roost Island"},
{158, "Pokemon Black and White - Accumula Town"},
{159, "Pokemon HeartGold and SoulSilver - Vermilion City"},
{160, "Undertale - Snowdin Town"},
{161, "Undertale - Bonetrousle"},
{162, "Undertale - Death by Glamour"},
{163, "Undertale - Home"},
{164, "Undertale - Ruins"},
{165, "Undertale - Spider Dance"},
{166, "Undertale - Waterfall"}
};




// Function to convert wide string (WCHAR) to std::string
std::string WideToString(const std::wstring& wideStr) {
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string result(bufferSize, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, &result[0], bufferSize, NULL, NULL);
    return result;
}

// Function to get the PID of Project64
DWORD GetProject64PID() {
   
    DWORD pid = 0;
    std::cout << "Project64 PID: " << pid << std::endl;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create process snapshot." << std::endl;
        return 0;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {
            std::string processName = WideToString(pe.szExeFile); // Convert WCHAR to std::string

            if (processName.find("Project64.exe") !=std::string::npos) {  // Adjust if your emulator has a different name
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    else {
        std::cerr << "Failed to retrieve process list." << std::endl;
    }

    CloseHandle(hSnapshot);
    return pid;
}

// Function to read memory from Project64
int readMemory(uintptr_t address) {
    int value = 0;
    SIZE_T bytesRead;

    DWORD pid = GetProject64PID();  // Call the function here
    if (pid == 0) {
        std::cerr << "Project64 process not found!" << std::endl;
        return -1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Failed to open Project64 process." << std::endl;
        return -1;
    }

    if (!ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(value), &bytesRead)) {
        std::cerr << "Failed to read memory at " << std::hex << address << std::endl;
    }

    CloseHandle(hProcess);
    return value;
}
// Configuration and state tracking structures
struct Config {
    std::string version = "1.1.1";
    std::string fontFace = "arial.ttf";
    bool showSongTitle = false;
    std::string backgroundImage = "(None)";
} config;

struct RunState {
    int attempts = 0;
    int pbStars = 0;
    int currentStars = 0;
    int currentSongID = 0;
    time_t startTime;
    time_t endTime;
    std::unordered_map<std::string, std::string> warpMap;
    std::unordered_map<std::string, int> starMap;
    std::string endReason;
};

RunState state;

void readAttemptsFile() {
    std::ifstream file("attempts.txt");
    if (file) {
        file >> state.attempts;
        file.close();
    }
}

void writeAttemptsFile() {
    std::ofstream file("attempts.txt");
    if (file) {
        file << state.attempts;
        file.close();
    }
}

void readPBStarsFile() {
    std::ifstream file("pb_stars.txt");
    if (file) {
        file >> state.pbStars;
        file.close();
    }
}

void writePBStarsFile() {
    std::ofstream file("pb_stars.txt");
    if (file) {
        file << state.pbStars;
        file.close();
    }
}

void readConfig() {
    std::ifstream file("config.json");
    if (file) {
        Json::Value root;
        file >> root;
        config.backgroundImage = root.get("BACKGROUND_IMAGE", "").asString(); // Default to empty string
        config.showSongTitle = root.get("SHOW_SONG_TITLE", false).asBool(); // Default to false
        file.close();
    }
}

void writeConfig() {
    Json::Value root;
    root["BACKGROUND_IMAGE"] = config.backgroundImage;
    root["SHOW_SONG_TITLE"] = config.showSongTitle;
    std::ofstream file("config.json");
    if (file) {
        Json::StreamWriterBuilder builder;
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(root, &file);
        file.close();
    }
}

#define MARIO_DEAD 0x00090013  // Example death state (modify as needed)
#define MARIO_SPAWNING 0x000C0000  // Example respawn state (modify as needed)

bool isDead = false;  // Track if Mario was in a death state

void checkForNewAttempt() {
    int marioAction = readMemory(CONFIG::MEM_MARIO::HURT_COUNTER);

    if (marioAction == MARIO_DEAD) {
        isDead = true;  // Mark Mario as dead
    }

    if (isDead && marioAction == MARIO_SPAWNING) {
        // Mario has respawned -> count as a new attempt
        state.attempts++;
        std::cout << "New attempt started! Total attempts: " << state.attempts << std::endl;
        writeAttemptsFile();  // Save immediately

        isDead = false;  // Reset death flag
    }
}

RECT GetProject64WindowRect() {
    HWND hwnd = FindWindow(NULL, L"Project64"); // Adjust if the window title is different
    RECT rect = { 0, 0, 800, 600 }; // Default size if not found

    if (hwnd) {
        GetWindowRect(hwnd, &rect);
    }
    else {
        std::cerr << "Project64 window not found!" << std::endl;
    }
    return rect;
}
void renderOverlay() {
    std::cout << "Creating overlay window..." << std::endl;

    // Create window (style set to None for transparency)
    sf::RenderWindow window(sf::VideoMode(800, 600), "IronMario Tracker Overlay", sf::Style::None);

    if (!window.isOpen()) {
        std::cerr << "Failed to create overlay window." << std::endl;
        return;
    }
    std::cout << "Overlay window successfully created." << std::endl;

    
    std::cout << "Window size: " << window.getSize().x << "x" << window.getSize().y << std::endl;
    std::cout << "Window position: " << window.getPosition().x << ", " << window.getPosition().y << std::endl;
    // Get window handle
    HWND hwnd = window.getSystemHandle();
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TOPMOST | WS_EX_LAYERED);
    std::cout << "Applied WS_EX_LAYERED." << std::endl;

    if (!SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY)) {
        std::cerr << "Failed to apply transparency! Error: " << GetLastError() << std::endl;
    }
    else {
        std::cout << "Transparency applied successfully." << std::endl;
    }


    // Load font
   
    sf::Font font;
    

    std::string fontPath = "C:\\Windows\\Fonts\\Arial.ttf";  // Full system path

    std::cout << "Loading font from: " << fontPath << std::endl;
    if (!font.loadFromFile(fontPath)) {
        std::cerr << "Error loading font from system path: " << fontPath << std::endl;
        return;
    }
    std::cout << "Font loaded successfully." << std::endl;

    

    
    // Test if pollEvent() is freezing
    std::cout << "Polling event before loop..." << std::endl;
    sf::Event event;
    window.pollEvent(event);  // Single event poll test
    std::cout << "Polling done." << std::endl;

    std::cout << "Entering render loop..." << std::endl;

    while (window.isOpen()) {
        std::cout << "Inside render loop..." << std::endl;
        RECT rect = GetProject64WindowRect();
        window.setPosition(sf::Vector2i(rect.left, rect.top)); // Align with Project64
        
        // Ensure position updates only if valid
        if (rect.right > rect.left && rect.bottom > rect.top) {
            window.setPosition(sf::Vector2i(rect.left, rect.top));
        }

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                std::cout << "Window close event detected!" << std::endl;
                window.close();
            }
            // Check if Project64 is still running
            if (GetProject64PID() == 0) {
                std::cout << "Project64 closed! Closing overlay." << std::endl;
                window.close();
            }
        }
        
        // If current run has more stars than PB, update pbStars
        if (state.currentStars > state.pbStars) {
            std::cout << "New PB detected! Updating pbStars from " << state.pbStars
                << " to " << state.currentStars << std::endl;
            state.pbStars = state.currentStars;
        }

        // Increment attempts when a new run starts
        state.attempts++;
        std::cout << "New attempt recorded. Total attempts: " << state.attempts << std::endl;
         
        // Read the current song ID & Get Current Star Count
        int currentSongID = readMemory(MEM::CURRENT_SONG_ID);
        state.currentStars = readMemory(CONFIG::MEM_HUD::STARS);

        // Find the song title in SONG_MAP
        std::string songName = "Unknown Song";
        auto it = CONFIG::MUSIC_DATA::SONG_MAP.find(currentSongID);
        if (it != CONFIG::MUSIC_DATA::SONG_MAP.end()) {
            songName = it->second;
        }
        
        sf::Text title("IronMario Tracker v" + config.version, font, 20);
        title.setPosition(10, 10);
        title.setFillColor(sf::Color::White);

        sf::Text attempts("Attempts: " + std::to_string(state.attempts), font, 18);
        attempts.setPosition(10, 40);
        attempts.setFillColor(sf::Color::White);

        sf::Text stars("Stars: " + std::to_string(state.currentStars), font, 18);
        stars.setPosition(10, 70);
        stars.setFillColor(sf::Color::Yellow);

        sf::Text pb("PB Stars: " + std::to_string(state.pbStars), font, 18);
        pb.setPosition(10, 100);
        pb.setFillColor(sf::Color::Green);
        std::cout << "Drawing text: " << title.getString().toAnsiString() << std::endl;
        // Update the song title text
       
        sf::Text songTitle("Song:" + songName, font, 18);
        songTitle.setPosition(10, 130);
        songTitle.setFillColor(sf::Color::Cyan);
        songTitle.setString("Song: " + songName);
        std::cout << "Rendering frame..." << std::endl;  // Debugging output

        

        
        window.clear(sf::Color::Transparent);
        window.draw(title);
        window.draw(attempts);
        window.draw(stars);
        window.draw(pb);
        window.draw(songTitle);
        window.display();
    }

    std::cout << "Waiting before closing..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "Exiting renderOverlay()..." << std::endl;
    }

    int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
        std::cout << "Starting IronMario Tracker..." << std::endl; // Debug message
#else
int main() {
#endif
    std::cout << "Starting IronMario Tracker..." << std::endl; // Debug message

    readAttemptsFile();
    readPBStarsFile();
    readConfig();

    
    state.startTime = time(0);

    std::cout << "Calling renderOverlay()..." << std::endl; // Debug message
    renderOverlay();
    std::cout << "Overlay closed." << std::endl; // Debug message

    state.endTime = time(0);
    writeConfig();
    writeAttemptsFile();
    writePBStarsFile();

    return 0;
}


