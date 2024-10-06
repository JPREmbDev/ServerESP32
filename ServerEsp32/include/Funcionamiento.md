### 1. **Conexión Wi-Fi (WiFiManager)**

Este es el responsable de manejar toda la lógica relacionada con la conexión Wi-Fi, incluyendo el uso de eventos para gestionar cuándo se conecta, desconecta o cuando se obtiene una dirección IP.

#### `wifi_manager.h` (Declaración de la clase `WiFiManager`)

```cpp
#pragma once

#include "esp_event.h"   // Para manejar los eventos del sistema
#include "esp_log.h"     // Para generar logs del sistema
#include "esp_wifi.h"    // Funciones y estructuras para la configuración Wi-Fi
#include <string>        // Para manejar cadenas de texto (std::string)

class WiFiManager {
private:
    std::string ssid;    // SSID de la red Wi-Fi
    std::string password; // Contraseña de la red Wi-Fi

public:
    WiFiManager(const std::string& ssid, const std::string& password); // Constructor para inicializar SSID y contraseña
    void connect(); // Método para conectarse a la red Wi-Fi
};
```

Aquí, se define la clase `WiFiManager`, que encapsula la funcionalidad relacionada con la conexión Wi-Fi. Tiene dos miembros privados (`ssid` y `password`) que almacenan el nombre y la contraseña de la red Wi-Fi. También se define el método `connect()` que gestiona la conexión.

#### `wifi_manager.cpp` (Implementación de la clase `WiFiManager`)

```cpp
#include "wifi_manager.h"
#include "esp_wifi.h"
#include "nvs_flash.h"  // Para la inicialización del sistema de almacenamiento No Volatile Storage (NVS)
#include "esp_event.h"
#include "esp_log.h"

// Etiqueta para logs
static const char* TAG = "WiFiManager";

// Manejador de eventos
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Wi-Fi iniciado, conectándose...");
        esp_wifi_connect(); // Conectar al Wi-Fi cuando el evento de inicio de STA se dispare
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Conexión Wi-Fi fallida, reintentando...");
        esp_wifi_connect(); // Intentar reconectar si la conexión falla
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Conexión exitosa, dirección IP: " IPSTR, IP2STR(&event->ip_info.ip)); // Log de la dirección IP obtenida
    }
}

WiFiManager::WiFiManager(const std::string& ssid, const std::string& password) : ssid(ssid), password(password) {
    // Constructor para inicializar el SSID y la contraseña
}

void WiFiManager::connect() {
    ESP_LOGI(TAG, "Iniciando NVS Flash");
    esp_err_t ret = nvs_flash_init(); // Inicialización del sistema de almacenamiento NVS
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase()); // Si no hay páginas libres o hay una nueva versión, borrar NVS
        ESP_ERROR_CHECK(nvs_flash_init()); // Reinicializar NVS
    }

    ESP_LOGI(TAG, "Inicializando la red TCP/IP");
    ESP_ERROR_CHECK(esp_netif_init()); // Inicialización de la pila TCP/IP

    ESP_LOGI(TAG, "Inicializando el loop de eventos");
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Inicialización del loop de eventos

    ESP_LOGI(TAG, "Inicializando configuración Wi-Fi");
    esp_netif_create_default_wifi_sta(); // Crear un adaptador Wi-Fi para STA (Station)

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // Configuración por defecto del Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // Inicializar Wi-Fi con la configuración predeterminada

    ESP_LOGI(TAG, "Registrando manejador de eventos Wi-Fi");
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL)); // Registrar el manejador de eventos Wi-Fi
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL)); // Registrar el manejador de eventos IP

    ESP_LOGI(TAG, "Configurando el modo Wi-Fi en modo estación (STA)");
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); // Configurar el Wi-Fi en modo estación (STA)

    wifi_config_t wifi_config = {};
    strncpy(reinterpret_cast<char*>(wifi_config.sta.ssid), ssid.c_str(), sizeof(wifi_config.sta.ssid)); // Configurar el SSID
    strncpy(reinterpret_cast<char*>(wifi_config.sta.password), password.c_str(), sizeof(wifi_config.sta.password)); // Configurar la contraseña

    ESP_LOGI(TAG, "Estableciendo configuración Wi-Fi con SSID: %s", ssid.c_str());
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config)); // Establecer la configuración Wi-Fi

    ESP_LOGI(TAG, "Iniciando Wi-Fi");
    ESP_ERROR_CHECK(esp_wifi_start()); // Iniciar el Wi-Fi
}
```

### Explicación de la Conexión Wi-Fi:

1. **Inicialización del NVS**: El sistema de almacenamiento no volátil (NVS) se inicializa para manejar la configuración Wi-Fi.
2. **Inicialización de la pila TCP/IP**: Se inicializa el stack TCP/IP para poder realizar conexiones de red.
3. **Manejador de eventos**: Los eventos del Wi-Fi (inicio, desconexión, obtención de IP) son manejados en el `wifi_event_handler`, que permite reaccionar cuando el Wi-Fi está listo o si algo falla.
4. **Configuración Wi-Fi**: Se configura el modo STA (Station) y se establece el SSID y la contraseña.
5. **Inicio de la conexión**: Se arranca el Wi-Fi, y si la conexión es exitosa, se obtiene una dirección IP.

### 2. **Servidor Web (WebServer)**

El servidor web es responsable de manejar las peticiones HTTP. Para simplificar, vamos a crear un servidor básico que escuche en un puerto y pueda responder a peticiones.

#### `server.h` (Declaración de la clase `WebServer`)

```cpp
#pragma once

#include "esp_http_server.h" // Librería para manejar el servidor HTTP

class WebServer {
private:
    httpd_handle_t server; // Manejador del servidor HTTP

public:
    WebServer();  // Constructor
    void start(); // Método para iniciar el servidor
    void stop();  // Método para detener el servidor
};
```

#### `server.cpp` (Implementación de la clase `WebServer`)

```cpp
#include "server.h"
#include "esp_log.h"

// Etiqueta para logs
static const char* TAG = "WebServer";

// Constructor de la clase WebServer
WebServer::WebServer() : server(nullptr) {}

// Función de manejador de solicitudes HTTP GET
esp_err_t hello_get_handler(httpd_req_t *req) {
    const char* resp_str = "Hola, este es el servidor web del ESP32!";
    httpd_resp_send(req, resp_str, strlen(resp_str)); // Enviar respuesta
    return ESP_OK;
}

// Método para iniciar el servidor web
void WebServer::start() {
    ESP_LOGI(TAG, "Iniciando servidor web");

    // Configuración del servidor HTTP
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Iniciar el servidor
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Servidor HTTP iniciado correctamente");

        // Configurar el manejador de la solicitud GET
        httpd_uri_t uri_get = {
            .uri      = "/hello",        // URI para acceder al recurso
            .method   = HTTP_GET,        // Método HTTP
            .handler  = hello_get_handler, // Función que maneja la petición
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server, &uri_get); // Registrar manejador del URI
    } else {
        ESP_LOGE(TAG, "Error al iniciar el servidor HTTP");
    }
}

// Método para detener el servidor web
void WebServer::stop() {
    if (server != nullptr) {
        httpd_stop(server); // Detener el servidor HTTP
        ESP_LOGI(TAG, "Servidor HTTP detenido");
    }
}
```

### Explicación del Servidor Web:

1. **Iniciar el servidor**: El método `start()` configura y arranca el servidor HTTP.
2. **Manejador de solicitudes GET**: El servidor responde a solicitudes HTTP GET en la URI `/hello`. La respuesta es un simple mensaje de texto.
3. **Detener el servidor**: El método `stop()` apaga el servidor si está en ejecución.

### 3. **Prueba del Servidor**

Después de cargar el código en el ESP32

 y conectarte a la red Wi-Fi, puedes abrir un navegador y dirigirte a `http://<ESP32_IP>/hello` para ver la respuesta del servidor web.

Este flujo muestra cómo crear una clase para la conexión Wi-Fi y otra para gestionar el servidor web, manteniéndolo modular y fácilmente extensible.