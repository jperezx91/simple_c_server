#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "hashmap.h"
#include "server.h"
#include "archivohttp.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h> 
#define BUFFER_SIZE 8068

// funciones
void flush_socket(int sock) {
    char buffer[1024];
    int bytes_available;

    // Mientras haya datos disponibles en el socket, lee y descarta
    while (ioctl(sock, FIONREAD, &bytes_available) == 0 && bytes_available > 0) {
        ssize_t received = recv(sock, buffer, sizeof(buffer), 0);
        if (received <= 0) {
            break; // Error o conexión cerrada
        }
    }
}

char checkArchivo(const char *recurso)
{
    int i = 0;
    while(recurso[i] != '\0')
    {
        if(recurso[i] == '.')
            return 0;
        i++;
    }
    return 1;
}

char *obtenerRecurso(const char *request)
{
    char *path = malloc(456);
    if (sscanf(request, "GET %255s", path) == 1) {
        
    } else {
        printf("Formato de la solicitud inválido.\n");
    }
    return path;
}
void notFound(int sock, const char *msg)
{
    char *rsp = malloc(strlen(msg) + 200);
    sprintf(rsp, "HTTP/1.1 404 Not Found\r\n\r\n %s", msg);
    send(sock, rsp, strlen(rsp), 0);
    close(sock);
}

void errReponse(int sock, const char *msg, char *err)
{
    char *rsp = malloc(strlen(msg) + 200);
    sprintf(rsp, "HTTP/1.1 %s\r\n\r\n %s", err, msg);
    send(sock, rsp, strlen(rsp), 0);
    close(sock);
}

void *handle_client(void *arg) {
    ClientData *client_data = (ClientData *)arg;
    int sock = *(int *)client_data->client_socket;
    free(client_data->client_socket);
    HashMap *map = client_data->map;
    char buffer[BUFFER_SIZE];
    ssize_t recieved = recv(sock, buffer, sizeof(buffer) - 1, 0); // Leer solicitud (no la procesamos realmente)
    printf("Status: %ld\n", recieved);
    if(recieved >= BUFFER_SIZE - 1)
    {
        flush_socket(sock);
        printf("Cerrando la conexion... %ld.\n", recieved);
        errReponse(sock, "La solicitud supero el limite permitido por el servidor.", "413 Content too large");
        close(sock);
    }
    char *direccion = obtenerRecurso(buffer);
    //printf("URL: %s | sock: %d\n", direccion, sock);
    HashNode *n = get(map, direccion);
    if(n == NULL)
    {
        // Nos fijamos que sea una direccion para buscar index.html
        if(strcmp(direccion, "/") == 0 || strlen(obtenerExtension(direccion)) <= 1)
        {
            if(direccion[strlen(direccion) - 1] == '/')
                n = get(map, strcat(direccion, "index.html"));
            else
                n = get(map, strcat(direccion, "/index.html")); // refactor dsp

            if(n == NULL)
            {
                if(client_data->react_mode == 1)
                {
                    printf("[REACT MODE] \n");
                    n = get(map, "/index.html");
                }else{
                    notFound(sock, "No existe recurso 1.");
                    if(direccion != NULL)
                        free(direccion);
                    pthread_exit(NULL);
                }
            }

        }else{
            notFound(sock, "No existe recurso 2.");
            free(direccion);
            pthread_exit(NULL);
        }
        
    }
    long size = n->sz;

    char *contentType = malloc(64);

    char *extension = obtenerExtension(direccion);
    // Refactor: usar un hashmap para almacenar los tipos de archivos.
     if(strcmp(extension, "html") == 0 || strcmp(extension, "htm") == 0){
        strcpy(contentType, "text/html");
    }else if(strcmp(extension, "css") == 0){
        strcpy(contentType, "text/css");
    }else if(strcmp(extension, "js") == 0){
        strcpy(contentType, "text/javascript");
    }else if(strcmp(extension, "png") == 0){
        strcpy(contentType, "image/png");
    }else if(strcmp(extension, "jpg") == 0 || strcmp(extension, "jpeg") == 0){
        strcpy(contentType, "image/jpeg");
    }else if(strcmp(extension, "ico") == 0){
        strcpy(contentType, "image/x-icon");
    }else if(strcmp(extension, "pdf") == 0){
        strcpy(contentType, "application/pdf");
    }else{
        strcpy(contentType, "text/html");
    }
    // Antes de enviar creamos la cabecera HTTP que se enviar al cliente
    int size_header = snprintf(NULL, 0, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", contentType, size);
    char *header = malloc(size_header + 1);
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", contentType, size);

    char *response = malloc(strlen(header) + n->sz + 1);
    int size_resposne = strlen(header) + n->sz + 1;
    memcpy(response, header, size_header);
    memcpy(response + size_header, n->value, n->sz);
    send(sock, response, size_resposne, 0); // Enviar respuesta

    close(sock); // Cerrar conexión
    pthread_exit(NULL);
}
