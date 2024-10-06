#include "server.h"
#include <esp_log.h>  // Para crear logs

static const char* TAG = "WebServer";  // Etiqueta para los mensajes de log

// Constructor del servidor web
WebServer::WebServer() : server_handle(nullptr) {}

// Destructor para detener el servidor si está corriendo
WebServer::~WebServer() {
    stop();  // Si el servidor está corriendo, lo detenemos
}

// Función estática que maneja las solicitudes GET en la ruta "/"
esp_err_t WebServer::handle_root_get(httpd_req_t* req) {
    const std::string response = "Hello from ESP32 using C++20!";
    ESP_LOGI(TAG, "Recibida solicitud GET en '/'");  // Log cuando se recibe una solicitud
    httpd_resp_send(req, response.c_str(), response.length());  // Enviar respuesta al cliente
    return ESP_OK;
}

// Inicializa el servidor HTTP
httpd_handle_t WebServer::init_server() {
    ESP_LOGI(TAG, "Configurando el servidor HTTP");
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();  // Configuración por defecto del servidor

    // Iniciamos el servidor
    if (httpd_start(&server_handle, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Servidor HTTP iniciado correctamente");

        // Definimos el manejador para la ruta "/"
        httpd_uri_t uri_get = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = handle_root_get,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server_handle, &uri_get);  // Registramos el manejador de la ruta "/"
    } else {
        ESP_LOGE(TAG, "Error al iniciar el servidor HTTP");
        return nullptr;
    }

    return server_handle;
}

// Iniciar el servidor
void WebServer::start() {
    if (!server_handle) {
        ESP_LOGI(TAG, "Iniciando servidor web");
        server_handle = init_server();  // Iniciamos el servidor si no está corriendo
    }
}

// Detener el servidor
void WebServer::stop() {
    if (server_handle) {
        ESP_LOGI(TAG, "Deteniendo servidor web");
        httpd_stop(server_handle);  // Detenemos el servidor
        server_handle = nullptr;
    }
}
