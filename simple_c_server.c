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
#include <signal.h>

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

void handle_exit(int sig) {
    if(sig == SIGINT)
    {
        printf("El servidor ha sido cerrado.\n");
        restaurarModoBloqueo();
        exit(0);
    }
}

void *handle_keyboard(void *arg) 
{
    ServerData *server_data = (ServerData *)arg;
    configurarModoSinBloqueo();
    char flag = 1;
    char c;
    while(flag){
        printf("Presione la tecla R para reiniciar el servidor. O presione Q para salir.\n");

        c = getchar();
        if(c == 'r')
        {
            printf("Reiniciando el servidor...\n");
            limpiar(server_data->map);
            sleep(1);
            cargarFileExplorer(server_data->map, server_data->extensionesModoLectura);
            leerDirectorio(server_data->folder, server_data->map, NULL, server_data->extensionesModoLectura);
        }
        if(c == 'q')
        {
            flag = 0;
            restaurarModoBloqueo();
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

    HashMap *extensionesFormato = create_map();
    HashMap *extensionesModoLectura = create_map();
    signal(SIGINT, handle_exit);

    char *folder  = malloc(256);
    char react_mode = 0;
    char interactive = 1;

    cargarFileExplorer(map, extensionesModoLectura);
    // TODO: Refactor manera de leer argumentos.
    if(argc < 2){   
        printf("Debes especificar el directorio que deseas usar como server web. Usando html como default.\n");
        leerDirectorio("html", map, NULL, extensionesModoLectura); // DEFAULT
        strcpy(folder, "html");
    }else{

    
    if(strcmp(argv[1], "--no-input") == 0)
    {
        interactive = 0;
        leerDirectorio("html", map, NULL, extensionesModoLectura); // DEFAULT
        strcpy(folder, "html");
    }
        
    else{
        leerDirectorio(argv[1], map, NULL, extensionesModoLectura);
        strcpy(folder, argv[1]);
    }
    
    }

    if(argc > 2)
    {
        if(strcmp(argv[2], "react") == 0)
            react_mode = 1;
        if(strcmp(argv[2], "--no-input") == 0)
            interactive = 0;
        
        if (argc > 3) {
            if(strcmp(argv[3], "--no-input") == 0)
                interactive = 0;
            
        }
    }


    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Formato de archivos soportados.
    insert(extensionesFormato, "html", "text/html\0", strlen("text/html"), NULL);
    insert(extensionesFormato, "htm", "text/html\0", strlen("text/html"), NULL);
    insert(extensionesFormato, "css", "text/css\0", strlen("text/css"), NULL);
    insert(extensionesFormato, "js", "text/javascript\0", strlen("text/javascript"), NULL);
    insert(extensionesFormato, "png", "image/png\0", strlen("image/png"), NULL);
    insert(extensionesFormato, "jpg", "image/jpeg\0", strlen("image/jpeg"), NULL);
    insert(extensionesFormato, "jpeg", "image/jpeg\0", strlen("image/jpeg"), NULL);
    insert(extensionesFormato, "ico", "image/x-icon\0", strlen("image/x-icon"), NULL);
    insert(extensionesFormato, "pdf", "application/pdf\0", strlen("application/pdf"), NULL);
    insert(extensionesFormato, "txt", "text/plain\0", strlen("text/plain"), NULL);
    insert(extensionesFormato, "json", "application/json\0", strlen("application/json"), NULL);
    insert(extensionesFormato, "zip", "application/zip\0", strlen("application/zip"), NULL);

    // Como leer archivos, (archivos de tipo texto)
    insert(extensionesModoLectura, "html", "r\0", strlen("text/html"), NULL);
    insert(extensionesModoLectura, "htm", "r\0", strlen("text/html"), NULL);
    insert(extensionesModoLectura, "txt", "r\0", strlen("text/plain"), NULL);
    insert(extensionesModoLectura, "css", "r\0", strlen("text/css"), NULL);
    insert(extensionesModoLectura, "js", "r\0", strlen("text/javascript"), NULL);


    // Crear socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error al configurar SO_REUSEADDR");
        close(server_fd);
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

    printf("[SIMPLE C SERVER]Servidor escuchando en el puerto %d...\n", PORT + tries);
    printf("[SIMPLE C SERVER] URL: http:localhost:%d...\n\n", PORT + tries);
    pthread_t thread_keyboard_id;
    ServerData *server_data = malloc(sizeof(ServerData));
    server_data->map = map;
    server_data->folder = folder;
    server_data->extensionesModoLectura = extensionesModoLectura;
    if(interactive == 1)
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
        client_data->extensionesFormato = extensionesFormato;
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