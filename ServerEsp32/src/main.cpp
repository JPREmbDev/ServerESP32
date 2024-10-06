#include "server.h"
#include "wifi_manager.h"
#include <esp_log.h>  // Para los logs

extern "C" void app_main() {
    ESP_LOGI("app_main", "Conectando al Wi-Fi");
    // Establecemos el SSID y la contraseña de la red Wi-Fi
    const std::string ssid = "Livebox6-2580";
    const std::string password = "B*t83re9%5o8^8itMNUoNQx@4Nu@*8SL";

    // Creamos una instancia del administrador de Wi-Fi
    WiFiManager wifi_manager(ssid, password);
    ESP_LOGI("app_main", "Conectando al Wi-Fi");
    wifi_manager.connect();  // Conectamos al Wi-Fi

    // Iniciamos el servidor web
    WebServer server;
    ESP_LOGI("app_main", "Iniciando el servidor web");
    server.start();  // Iniciamos el servidor web
        // Mantén el programa ejecutándose
    while (true) 
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
