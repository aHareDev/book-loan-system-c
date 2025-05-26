#ifndef UTILS_H
#define UTILS_H

#include "entities.h"

int readRequest(int fd_read, Request *req);
int sendResponse(Request *req, char *response);
int loadLibraryFromFile(Library *db, char *filename);
int saveLibraryToFile(Library *lib, const char *filename);
void library_printReports(Library *library);
void showRequest(Request *req);

#endif
