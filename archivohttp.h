#include "hashmap.h"
#pragma once
typedef struct ArchivoHttp {
    char *archv;
    int size;
    char flag_dir;
} ArchivoHttp;


ArchivoHttp *leerArchivo(char *archivo, HashMap *extensionesModoLectura);
void leerDirectorio(const char *directorio, HashMap *map, HashNode *parent, HashMap *extensionesModoLectura);
char *eliminarPrimerDirectorio(char *str);
char *obtenerExtension(const char *archivo);
void cargarFileExplorer(HashMap *map, HashMap *extensionesModoLectura);