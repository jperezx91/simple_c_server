#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include "archivohttp.h"
#include <dirent.h>
#include <sys/stat.h>



ArchivoHttp *leerArchivo(char *archivo) {
    char *openMode = malloc(3);
    // Refactor, usar un HashMap para almacenar los tipos de archivos.
    if(strcmp(obtenerExtension(archivo), "html") == 0 ||
       strcmp(obtenerExtension(archivo), "htm") == 0  ||
       strcmp(obtenerExtension(archivo), "txt") == 0  ||
       strcmp(obtenerExtension(archivo), "css") == 0  ||
       strcmp(obtenerExtension(archivo), "js") == 0)
    {
        strcpy(openMode, "r");
    }else{
        strcpy(openMode, "rb");
    }
    FILE *file = fopen(archivo, openMode);
    if (file == NULL) {
        perror("Error al abrir archivo");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    printf("fileName: %s | Size: %ld\n",archivo, size);
    rewind(file);
    char *bf = malloc(size + 1);
    fread(bf, 1, size, file);
    if(strcmp(openMode, "r") == 0)
    {
        bf[size] = '\0';
    }
    fclose(file);
    ArchivoHttp *output = malloc(sizeof(ArchivoHttp));
    output->size = size;
    if(strcmp(openMode, "r") == 0)
    {
        output->archv = malloc(size);
        memcpy(output->archv, bf, size);
    }else{
        output->archv = malloc(size);
        memcpy(output->archv, bf, size);
    }
    free(bf);
    return output;
}

void leerDirectorio(const char *directorio, HashMap *map) {
    const char *folder_path = directorio; // Ruta de la carpeta a leer (cambiar según necesidad)
    struct dirent *entry;
    DIR *dir = opendir(folder_path);
    
    if (dir == NULL) {
        perror("No se pudo abrir el directorio");
    }

    struct stat file_stat;
    char full_path[1024]; // Para almacenar la ruta completa




    while ((entry = readdir(dir)) != NULL) {
        // Construir la ruta completa del archivo
        snprintf(full_path, sizeof(full_path), "%s/%s", folder_path, entry->d_name);

        // Usar stat para obtener información sobre el archivo
        if (stat(full_path, &file_stat) == 0) {
            // Verificar si es un archivo regular
            if (S_ISREG(file_stat.st_mode)) {
                printf("- %s/%s (dir: %s)\n", directorio, entry->d_name, eliminarPrimerDirectorio(full_path));
                ArchivoHttp *miarchivo = leerArchivo(full_path);
                insert(map, eliminarPrimerDirectorio(full_path), miarchivo->archv, miarchivo->size);
            }else{
                if(S_ISDIR(file_stat.st_mode)){
                    
                    if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
                        leerDirectorio(full_path, map);
                    }
                    
                }
            }
        } else {
            perror("Error al obtener información del archivo");
        }
    }

    closedir(dir);
}

char *eliminarPrimerDirectorio(char *str)
{
    char *output = malloc(strlen(str) + 2);
    int i = 0;
    int j = 1;
    char startcpy = 0;
    output[0] = '/';
    while(str[i] != '\0')
    {
        if(startcpy == 1)
        {
            output[j] = str[i];
            j++;
        }
        if(str[i] == '/')
            startcpy = 1;
        i++;
    }
    output[j] = '\0';
    return output;
}

char *obtenerExtension(const char *archivo)
{
    char *extension = malloc(8);
    int i = 0;
    int j = 0;
    char flagPunto = 0;
    while(archivo[i] != '\0')
    {
        if(flagPunto == 1)
        {
            extension[j] = archivo[i];
            j++;
        }
        if(archivo[i] == '.')
            flagPunto = 1;
        i++;
    }
    extension[j] = '\0';
    return extension;
}