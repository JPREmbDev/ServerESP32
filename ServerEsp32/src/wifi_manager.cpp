#include "wifi_manager.h"
#include <esp_wifi.h>        // Librería para manejar el Wi-Fi en ESP32
#include <esp_log.h>         // Librería para generar registros de log
#include <nvs_flash.h>       // Librería para manejar la memoria flash (almacenamiento no volátil)
#include <cstring>           // Para usar strncpy (funciones de manipulación de cadenas en C)
#include <esp_event.h>       // Para manejar eventos del sistema
#include <esp_netif.h>       // Para obtener la IP

static const char* TAG = "WiFiManager"; // Etiqueta para los mensajes de log

// Callback para manejar los eventos Wi-Fi
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Wi-Fi iniciado, conectándose...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Conexión Wi-Fi fallida, reintentando...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Conexión exitosa, dirección IP: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Iniciando servidor web...");
    }
}


// Constructor que recibe el SSID y la contraseña de la red Wi-Fi
WiFiManager::WiFiManager(const std::string& ssid, const std::string& password)
    : ssid(ssid), password(password) {}

// Método para conectar el ESP32 a la red Wi-Fi
void WiFiManager::connect() {
    // Inicia el sistema de almacenamiento no volátil (NVS)
    ESP_LOGI(TAG, "Iniciando NVS Flash");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS necesita ser borrado");
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Inicializar la pila de red TCP/IP
    ESP_LOGI(TAG, "Inicializando la red TCP/IP");
    esp_netif_init();  // Inicializa el adaptador de red (pila TCP/IP)

    // Crear el bucle de eventos por defecto
    ESP_LOGI(TAG, "Inicializando el loop de eventos");
    esp_event_loop_create_default(); 

    // Crear una interfaz de red Wi-Fi predeterminada (modo estación)
    esp_netif_create_default_wifi_sta();

    // Inicializa la configuración Wi-Fi
    ESP_LOGI(TAG, "Inicializando configuración Wi-Fi");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Registrar el manejador de eventos para los eventos Wi-Fi y de IP
    ESP_LOGI(TAG, "Registrando manejador de eventos Wi-Fi");
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);

    // Configurar el modo Wi-Fi en estación (STA)
    ESP_LOGI(TAG, "Configurando el modo Wi-Fi en modo estación (STA)");
    esp_wifi_set_mode(WIFI_MODE_STA);

    // Configuración del SSID y la contraseña
    ESP_LOGI(TAG, "Configurando SSID y contraseña");
    wifi_config_t wifi_config = {};
    strncpy(reinterpret_cast<char*>(wifi_config.sta.ssid), ssid.c_str(), sizeof(wifi_config.sta.ssid));
    strncpy(reinterpret_cast<char*>(wifi_config.sta.password), password.c_str(), sizeof(wifi_config.sta.password));

    // Establecemos la configuración del Wi-Fi
    ESP_LOGI(TAG, "Estableciendo configuración Wi-Fi con SSID: %s", ssid.c_str());
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

    // Inicia el Wi-Fi
    ESP_LOGI(TAG, "Iniciando Wi-Fi");
    esp_wifi_start();  // Inicia el Wi-Fi

    // Conectarse a la red Wi-Fi
    ESP_LOGI(TAG, "Conectándose a la red Wi-Fi");
    esp_wifi_connect();
}
