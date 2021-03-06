#include "ConfigManager.hpp"

namespace Cycleshooter {

ConfigManager::ConfigManager() {
    std::string configFile = "config.toml";

    std::ifstream ifs(configFile);
    if(!ifs) {
        LOG_FATAL("No such file exists: %s", configFile.c_str());
    }

    toml::Parser parser(ifs);
    v = parser.parse();

    if(!v.valid()) {
        LOG_FATAL("Couldn't parse the %s config file: %s", configFile.c_str(), parser.errorReason().c_str());
    }
}

std::string ConfigManager::getStr(const std::string& key) const {
    return get<std::string>(key);
}

int ConfigManager::getInt(const std::string& key) const {
    return get<int>(key);
}

double ConfigManager::getDouble(const std::string& key) const {
    return get<double>(key);
}

bool ConfigManager::getBool(const std::string &key) const {
    return get<bool>(key);
}

}
