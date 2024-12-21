#include "hashmap.h"
#pragma once
typedef struct ArchivoHttp {
    char *archv;
    int size;
} ArchivoHttp;

ArchivoHttp *leerArchivo(char *archivo);
void leerDirectorio(const char *directorio, HashMap *map);
char *eliminarPrimerDirectorio(char *str);
char *obtenerExtension(const char *archivo);
