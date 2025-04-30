
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


void sreplace(const char *str, const char *search, const char *replace, char *output) {
    const char *pos = str;
    const size_t len_search = strlen(search);
    const size_t len_replace = strlen(replace);
    size_t output_pos = 0;

    while ((pos = strstr(pos, search)) != NULL) {
        // Copiar el texto antes de 'search' al output
        size_t prefix_len = pos - str;
        strncpy(output + output_pos, str, prefix_len);
        output_pos += prefix_len;

        // Copiar 'replace' al output
        strncpy(output + output_pos, replace, len_replace);
        output_pos += len_replace;

        // Avanzar el puntero más allá de 'search'
        pos += len_search;
        str = pos;
    }

    // Copiar el resto de la cadena original al output
    strcpy(output + output_pos, str);
}


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
    if(client_data->client_socket != NULL)
        free(client_data->client_socket);
    HashMap *map = client_data->map;
    HashMap *extensionesFormato = client_data->extensionesFormato;

    char buffer[BUFFER_SIZE];
    ssize_t recieved = recv(sock, buffer, sizeof(buffer) - 1, 0); // Leer solicitud (no la procesamos realmente)
    if(recieved >= BUFFER_SIZE - 1)
    {
        flush_socket(sock);
        printf("Cerrando la conexion... %ld.\n", recieved);
        errReponse(sock, "La solicitud supero el limite permitido por el servidor.", "413 Content too large");
        close(sock);
    }
    char *direccion = obtenerRecurso(buffer);
    int sz_direccion = strlen(direccion);
    // Parseamos previamente la direccion, por si tiene una contrabarra al final.
    if(direccion[sz_direccion - 1] == '/')
        direccion[sz_direccion - 1] = '\0';

    if(strcmp(direccion, "") == 0)
    {
        strcpy(direccion, "/index.html");
    }
    

    HashNode *n = get(map, direccion);
    char *bffresponse = NULL;
    HashNode *aux;

    // Primero nos fijamos si existe el recurso
    if(n != NULL)
    {
        HashNode *anx = n->children;
        while(anx != NULL)
        {
            anx = anx->nextChild;
        }
        if (n->sz == -1) // Es un directorio
        {
            // Nos fijamos si ese directorio tiene un index.html
            char *cpydirection = malloc(strlen(direccion) + 1);
            strcpy(cpydirection, direccion);
            aux = get(map, strcat(cpydirection, "/index.html"));
            if(aux != NULL)
            {
                n = aux;
            }else{
                HashNode *anx = n->children; // esto listará todos los archivos y directorios del recurso actual.
                HashNode *anx2 = n->children;
                int tsize_files = 0;
                while(anx != NULL)
                {
                    tsize_files += strlen(anx->key) + 1;
                    anx = anx->nextChild;
                }
                char *files_directory = calloc(tsize_files, sizeof(char));
                files_directory[0] = '\0';
                while(anx2 != NULL)
                {
                    strcat(files_directory, anx2->key);
                    if(anx2->nextChild != NULL)
                        strcat(files_directory, "%");
                    tsize_files += strlen(anx2->key) + 1;
                    anx2 = anx2->nextChild;
                }
               n = get(map, "%_fexplorer.html");
               char *bffresponse_prev = calloc(n->sz + (strlen(direccion) * 2), sizeof(char));
               sreplace(n->value, "%DIRECTORY%", direccion, bffresponse_prev);
               bffresponse_prev[strlen(bffresponse_prev) - 1] = '\0';
               bffresponse = calloc(n->sz + (strlen(direccion) * 2) + strlen(files_directory) + 1, sizeof(char));
               sreplace(bffresponse_prev, "%FILESDIR%", files_directory, bffresponse);
               free(files_directory);
               bffresponse[strlen(bffresponse) - 1] = '\0';
            }

        }
    }else{
        notFound(sock, "No existe el recurso.");
        close(sock); // Cerrar conexión
        pthread_exit(NULL);
    }
    long size;
    if(bffresponse == NULL)
    {
        bffresponse = malloc(n->sz);
        memcpy(bffresponse, n->value, n->sz);
        size = n->sz;
    }else{
        size = strlen(bffresponse);
    }
    
    char *contentType = malloc(128);

    char *extension = obtenerExtension(direccion);
    HashNode *extensionNode = get(extensionesFormato, extension);
    if(extensionNode == NULL)
    {
        strcpy(contentType, "text/html");
    }else{
        strcpy(contentType, extensionNode->value);
    }
    
    // Antes de enviar creamos la cabecera HTTP que se enviar al cliente
    int size_header = snprintf(NULL, 0, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", contentType, size);
    char *header = malloc(size_header + 1);
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", contentType, size);

    char *response = malloc(strlen(header) + size + 1);
    int size_resposne = strlen(header) + size + 1;
    memcpy(response, header, size_header);
    memcpy(response + size_header, bffresponse, size);
    send(sock, response, size_resposne, 0); // Enviar respuesta
    free(header);
    free(response);
    if(contentType != NULL)
        free(contentType);
    if(extension != NULL)
        free(extension);
    
    memset(bffresponse, 0, size);
    if(bffresponse != NULL)
        free(bffresponse);
    bffresponse = NULL;
    close(sock); // Cerrar conexión
    pthread_exit(NULL);
        
    
    
}
