#pragma once
#include <string>

class GameMeta {
public:
    GameMeta() = default;
    ~GameMeta() = default;

    bool loadFromFile(const std::string& filename);

    const std::string& getTitle() const { return title; }
    const std::string& getVersion() const { return version; }
    const std::string& getDirectory() const { return directory; }

private:
    std::string title;
    std::string version;
    std::string directory;
};