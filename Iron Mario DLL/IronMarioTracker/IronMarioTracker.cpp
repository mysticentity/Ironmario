#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <ctime>
#include <json/json.h> // JSON handling library
#include <windows.h>
#ifdef _WIN32



// Configuration and state tracking structures
struct Config {
    std::string version = "1.1.1";
    std::string fontFace = "arial.ttf";
    bool showSongTitle = false;
    std::string backgroundImage = "(None)";
} config;

struct RunState {
    RunState() : startTime(0), endTime(0), attempts(0), pbStars(0), currentStars(0), endReason("") {}
    int attempts = 0;
    int pbStars = 0;
    int currentStars = 0;
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

void renderOverlay() {
    std::cout << "Entering renderOverlay()..." << std::endl; // Debug

    sf::RenderWindow window(sf::VideoMode(400, 250), "IronMario Tracker Overlay", sf::Style::Default);
    window.setPosition(sf::Vector2i(100, 100));
    window.setVisible(true);
    window.requestFocus();

    std::cout << "Overlay window created." << std::endl; // Debug

    sf::Font font;
    if (!font.loadFromFile(config.fontFace)) {
        std::cerr << "Error loading font!" << std::endl;
        return;
    }

    std::cout << "Font loaded successfully." << std::endl; // Debug


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

    sf::Text songTitle("Song: Not Available", font, 18);
    songTitle.setPosition(10, 130);
    songTitle.setFillColor(sf::Color::Cyan);

    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Transparent);
        window.draw(title);
        window.draw(attempts);
        window.draw(stars);
        window.draw(pb);
        if (config.showSongTitle) {
            window.draw(songTitle);
        }
        window.display();
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    std::cout << "Starting IronMario Tracker..." << std::endl; // Debug message
#else
int main() {
#endif
    readAttemptsFile();
    readPBStarsFile();
    readConfig();

    state.currentStars = 5;
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