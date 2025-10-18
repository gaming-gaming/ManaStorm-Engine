#pragma once
#include <string>

class EngineConfig {
public:
    EngineConfig() = default;
    ~EngineConfig() = default;

    bool loadFromFile(const std::string& filename);

    const std::string& getDisplayMode() const { return displayMode; }
    int getDisplayIndex() const { return displayIndex; }
    int getResolutionWidth() const { return resolutionWidth; }
    int getResolutionHeight() const { return resolutionHeight; }
    const std::string& getApi() const { return api; }
    int getFrameRateLimit() const { return frameRateLimit; }
    bool isVsyncEnabled() const { return vsync; }

private:
    std::string displayMode;
    int displayIndex = 0;
    int resolutionWidth = 0;
    int resolutionHeight = 0;
    std::string api;
    int frameRateLimit = 0;
    bool vsync = true;
};