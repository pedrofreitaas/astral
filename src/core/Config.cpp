#include "Config.h"
#include <fstream>
#include <iostream>

bool Config::Initialize(const std::string& configPath) {
    std::string basePath = "../assets/Config/";
    
    mFilePath = basePath + configPath;
    std::ifstream file(mFilePath);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open config: " << configPath << std::endl;
        return false;
    }
    
    try {
        file >> mData;
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }
    
    return true;
}

void Config::Update() {
    // future
}

template<typename T>
T Config::Get(const std::string& key) const {
    nlohmann::json current = mData;
    
    size_t start = 0;
    while (start < key.length()) {
        size_t dot = key.find('.', start);
        size_t end = (dot == std::string::npos) ? key.length() : dot;
        std::string part = key.substr(start, end - start);
        
        // Convert to lowercase for case-insensitive matching
        std::transform(part.begin(), part.end(), part.begin(), ::toupper);
        
        if (current.contains(part)) {
            current = current[part];
        } else {
            throw std::runtime_error("Key not found: " + part);
        }
        
        start = end + 1;
    }
    
    return current.get<T>();
}

template int Config::Get<int>(const std::string&) const;
template float Config::Get<float>(const std::string&) const;
template double Config::Get<double>(const std::string&) const;
template bool Config::Get<bool>(const std::string&) const;
template std::string Config::Get<std::string>(const std::string&) const;