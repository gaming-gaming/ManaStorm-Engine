#include "EngineConfig.hpp"

#include <fstream>
#include <windows.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool EngineConfig::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::string errorMsg = "Unable to locate " + filename;
        MessageBoxA(nullptr, errorMsg.c_str(), "Fatal Error", MB_ICONERROR);
        ExitProcess(1);
    }

    json j;
    try {
        file >> j;
        auto& display = j["graphics"]["display"];
        this->displayMode = display.value("displayMode", "windowed");
        this->displayIndex = display.value("displayIndex", 0);
        this->resolutionWidth = display["resolution"][0].is_null() ? 0 : display["resolution"][0].get<int>();
        this->resolutionHeight = display["resolution"][1].is_null() ? 0 : display["resolution"][1].get<int>();
        this->api = display.value("api", "openGL");
        this->frameRateLimit = display["frameRateLimit"].is_null() ? 0 : display["frameRateLimit"].get<int>();
        this->vsync = display.value("vsync", true);
    } catch (json::parse_error& e) {
        std::string errorMsg = "Unable to parse " + filename;
        MessageBoxA(nullptr, errorMsg.c_str(), "Fatal Error", MB_ICONERROR);
        ExitProcess(1);
    }

    return true;
}