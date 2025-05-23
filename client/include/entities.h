#ifndef COMMON_H
#define COMMON_H

typedef enum {
    DEVOLVER,
    RENOVAR,
    PRESTAR,
    SALIR
} Operation;

typedef struct {
    Operacion type;
    char bookTitle[100];
    int isbn;
    pid_t pid; // para identificar qué proceso solicitante envió la solicitud
} Request;

typedef struct {
    int code; // 0 = error, 1 = success
    char message[100];
} Response;

typedef struct {
    int id;
    int isbn;
    char status; // 'D' o 'P'
    char date[20];
} Copy;

typedef struct {
    char title[100];
    int isbn;
    int totalCopies;
    Copy copies[MAX_COPIES];
} Book;

#endif
