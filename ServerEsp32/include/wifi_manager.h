#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <string>

class WiFiManager {
public:
    WiFiManager(const std::string& ssid, const std::string& password);
    void connect();

private:
    std::string ssid;
    std::string password;
};

#endif
