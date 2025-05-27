#ifndef UTILS_H
#define UTILS_H

#include "entities.h"

// Lee una solicitud desde un descriptor de archivo.
int readRequest(int fd_read, Request *req);

// Envía una respuesta al proceso solicitante.
int sendResponse(Request *req, char *response);

// Carga los datos de la biblioteca desde un archivo.
int loadLibraryFromFile(Library *db, char *filename);

// Guarda los datos de la biblioteca en un archivo.
int saveLibraryToFile(Library *lib, const char *filename);

// Imprime los reportes almacenados en la biblioteca.
void library_printReports(Library *library);

// Muestra información detallada de una solicitud recibida.
void showRequest(Request *req);

#endif
