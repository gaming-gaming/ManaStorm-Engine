#include "GameMeta.hpp"

#include <fstream>
#include <windows.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool GameMeta::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::string errorMsg = "Unable to locate " + filename;
        MessageBoxA(nullptr, errorMsg.c_str(), "Fatal Error", MB_ICONERROR);
        ExitProcess(1);
    }

    json j;
    try {
        file >> j;
        title = j["title"];
        version = j["version"];
        directory = j["directory"];
    } catch (json::parse_error& e) {
        std::string errorMsg = "Unable to parse " + filename;
        MessageBoxA(nullptr, errorMsg.c_str(), "Fatal Error", MB_ICONERROR);
        ExitProcess(1);
    }

    return true;
}