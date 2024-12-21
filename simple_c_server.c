#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include "archivohttp.h"
#include "server.h"
#include <unistd.h>
#include <termios.h>
#define PORT 8080
#define BUFFER_SIZE 1024

void configurarModoSinBloqueo() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t); // Obtiene la configuración actual del terminal
    t.c_lflag &= ~(ICANON | ECHO); // Desactiva el modo canon y el eco
    tcsetattr(STDIN_FILENO, TCSANOW, &t); // Aplica los cambios
}

void restaurarModoBloqueo() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= (ICANON | ECHO); // Reactiva el modo canon y el eco
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}


void *handle_keyboard(void *arg) 
{
    ServerData *server_data = (ServerData *)arg;
    configurarModoSinBloqueo();
    char flag = 1;
    while(flag){
        
        printf("Presione la tecla R para reiniciar el servidor. O presione Q para salir.\n");

        char c = getchar();
        if(c == 'r')
        {
            printf("Reiniciando el servidor...\n");
            limpiar(server_data->map);
            //server_data->map = create_map();
            sleep(1);
            leerDirectorio(server_data->folder, server_data->map);
        }
        if(c == 'q')
        {
            flag = 0;
            exit(0);
        }
        fflush(stdin);
        
        sleep(1);
    }
    restaurarModoBloqueo() ;
    return NULL;
    
}
int main(int argc, char *argv[])
{
    HashMap *map = create_map();

    char *folder  = malloc(256);
    char react_mode = 0;
    if(argc < 2){   
        printf("Debes especificar el directorio que deseas usar como server web. Usando html como default.\n");
        leerDirectorio("html", map); // DEFAULT
        strcpy(folder, "html");
    }else{

    leerDirectorio(argv[1], map);
    strcpy(folder, argv[1]);
    }

    if(argc > 2)
    {
        if(strcmp(argv[2], "react") == 0)
            react_mode = 1;
    }


    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Crear socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear socket");
        exit(EXIT_FAILURE);
    }

    // Configurar dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Asignar socket al puerto
    int tries = 0;
    while (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 && tries < 5) {

        tries++;
        server_addr.sin_port = htons(PORT + tries);
    }
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 && tries == 5) 
    {
        perror("Error en bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones
    if (listen(server_fd, 10) < 0) {
        perror("Error en listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en el puerto %d...\n", PORT + tries);
    pthread_t thread_keyboard_id;
    ServerData *server_data = malloc(sizeof(ServerData));
    server_data->map = map;
    server_data->folder = folder;
    if(pthread_create(&thread_keyboard_id, NULL, handle_keyboard, (void *)server_data) != 0) { // cuando presionamos R, vuelve a leer el directorio del servidor.
        perror("Error al crear hilo");
    }
    while (1) {
        // Aceptar nueva conexión
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
            perror("Error en accept");
            continue;
        }

        // Crear un hilo para manejar al cliente
        pthread_t thread_id;
        int *client_sock = malloc(sizeof(int));
        *client_sock = client_fd;

        ClientData *client_data = malloc(sizeof(ClientData));
        client_data->client_socket = client_sock;
        client_data->map = map;
        client_data->react_mode = react_mode;

        if (pthread_create(&thread_id, NULL, handle_client, (void *)client_data) != 0) {
            perror("Error al crear hilo");
            close(client_fd);
            free(client_sock);
        }

        pthread_detach(thread_id); // Liberar recursos del hilo al finalizar
    }

    close(server_fd);
    return 0;
}