#ifndef COMMON_H
#define COMMON_H

#define PIPE_NAME_MAX 50
#define NAME_MAX 100
#define BUFFER_SIZE 256

typedef struct {
        char operation;
        char title[NAME_MAX];
        int isbn;
        pid_t pid; // para identificar qué proceso solicitante envió la solicitud
        char pipeResponse[PIPE_NAME_MAX];
} Request;

typedef struct {
        int code;
        char message[BUFFER_SIZE];
} Response;

#endif
