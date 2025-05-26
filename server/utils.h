#ifndef UTILS_H
#define UTILS_H

#include "entities.h"

int readRequest(int fd_read, Request *req);
int sendResponse(Request *req);
int loadLibraryFromFile(Library *db, char *filename);
void mostrarSolicitud(Request *req);

#endif
