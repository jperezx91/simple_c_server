
# Simple C HTTP Server

Este proyecto implementa un servidor HTTP simple en C. Permite servir archivos estáticos desde un directorio especificado y explorar directorios a través de un navegador web. 

## Características

-   Servir archivos estáticos como HTML, CSS, JavaScript, imágenes, y más.
-   Exploración de directorios mediante una interfaz HTML generada dinámicamente.
-   Soporte para múltiples conexiones utilizando threads.
-   Modo interactivo para reiniciar el servidor o cerrarlo desde la terminal.
-   Configuración de extensiones y tipos MIME soportados.

## Estructura del Proyecto

.gitignore  
archivohttp.c  
archivohttp.h  
hashmap.c  
hashmap.h  
LICENSE  
makefile  
README.md  
server.c  
server.h  
simple_c_server.c  
.vscode/  
fexplorer/  
html/

-   **`simple_c_server.c`**: Archivo principal que inicializa el servidor.
-   **`server.c`  y  server.h**: Manejo de conexiones y solicitudes HTTP.
-   **`archivohttp.c`  y  archivohttp.h**: Funciones para leer archivos y directorios.
-   **`hashmap.c`  y  hashmap.h**: Implementación de un HashMap para almacenar datos del servidor.
-   **html**: Archivos estáticos servidos por el servidor.
-   **fexplorer**: Plantilla HTML para explorar directorios.

## Requisitos

-   GCC (GNU Compiler Collection)
-   Sistema operativo basado en Unix (Linux o macOS recomendado)

## Compilación

Para compilar el proyecto, utiliza el  makefile  incluido:

make

Esto generará un ejecutable llamado  `simple_c_server`.

## Uso

Ejecuta el servidor con el siguiente comando:

./simple_c_server [directorio] [opciones]

-   **`directorio`**: Directorio raíz para servir archivos (por defecto,  html).
-   **Opciones**:
    -   `--no-input`: Desactiva el modo interactivo.
    -   `react`: Activa el modo React (experimental, hace que todas las URL 404 redireccionen a index.html).

### Ejemplo

./simple_c_server html

Accede al servidor desde tu navegador en  `http://localhost:8080`.

## Funcionalidades Interactivas

-   Presiona  `R`  en la terminal para reiniciar el servidor y recargar los archivos.
-   Presiona  `Q`  para cerrar el servidor.

## Extensiones Soportadas

El servidor soporta los siguientes tipos de archivos:

-   HTML, CSS, JS
-   Imágenes: PNG, JPG, JPEG, ICO
-   Archivos de texto: TXT, JSON
-   Archivos comprimidos: ZIP
-   PDF

## Exploración de Directorios

Si un directorio no contiene un archivo  `index.html`, el servidor genera una lista de archivos y subdirectorios utilizando la plantilla en  fexplorer_files.html.


## Licencia

Este proyecto está licenciado bajo la Licencia Apache 2.0. Consulta el archivo  LICENSE  para más detalles.
