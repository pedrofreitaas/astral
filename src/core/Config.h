#pragma once
#include <string>
#include <optional>
#include "../libs/Json.h"

class Config {
public:
    Config() = default;
    ~Config() = default;

    // Initialize configuration from a JSON file, basePath: 'assets/Config/'
    bool Initialize(const std::string& configPath);
    void Update();
    
    template<typename T>
    T Get(const std::string& key) const;

private:
    nlohmann::json mData;
    std::string mFilePath;
};