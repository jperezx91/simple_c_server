#pragma once

typedef struct ClientData {
    void *client_socket;
    HashMap *map;
    HashMap *extensionesFormato;
    char react_mode;

}ClientData;

typedef struct ServerData {
    HashMap *map;
    char *folder;
     HashMap *extensionesModoLectura;
} ServerData;
char checkArchivo(const char *recurso);
char *obtenerRecurso(const char *request);
void notFound(int sock, const char *msg);
void *handle_client(void *arg);
