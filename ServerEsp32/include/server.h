#ifndef SERVER_H
#define SERVER_H

#include <esp_http_server.h>
#include <string>

class WebServer {
public:
    WebServer();
    ~WebServer();
    
    // Método para iniciar el servidor
    void start();

    // Método para detener el servidor
    void stop();

private:
    httpd_handle_t server_handle;
    
    // Configuración del servidor
    httpd_handle_t init_server();

    // Funciones estáticas para manejar las solicitudes
    static esp_err_t handle_root_get(httpd_req_t* req);
};

#endif
