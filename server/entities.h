#ifndef COMMON_H
#define COMMON_H

#include <semaphore.h>

#define PIPE_NAME_SIZE 50
#define NAME_SIZE 100
#define MAX_CHARACTERS 256
#define ITEMS_BUFFER 10
#define MAX_BOOKS 30

typedef struct {
    	char operation;
    	char title[NAME_SIZE];
    	int isbn;
    	pid_t pid; // para identificar qué proceso solicitante envió la solicitud
	char pipeResponse[PIPE_NAME_SIZE];
} Request;

typedef struct {
	int code;
	char message[MAX_CHARACTERS];
} Response;

typedef struct {
	Request items[ITEMS_BUFFER];
	int in;
	int out;
	pthread_mutex_t mutex;
	sem_t full;
	sem_t empty;
} Buffer;

typedef struct {
	char status;
	char bookName[MAX_CHARACTERS];
	int isbn;
	int idCopy;
	char date[30];
} Report;

typedef struct {
	int id;
	int isbn;
	char status; // 'D' o 'P'
	char date[30];
} Copy;

typedef struct {
	char title[MAX_CHARACTERS];
	int isbn;
	int totalCopies;
	Copy copies[ITEMS_BUFFER];
} Book;

typedef struct {
	char fileOutput[MAX_CHARACTERS];
     int redibujar_menu;
     pthread_mutex_t redibujar_mutex;
        Buffer *buf;
        Book books[MAX_BOOKS];
	Report reports[NAME_SIZE];
        int *ejecutando;
	pthread_mutex_t ejecutando_mutex;
	pthread_mutex_t print_mutex;
} Library;

void library_addReport(Library *library, char status, const char *title, int isbn, int idCopy, const char *date);
Book *library_findBookByISBN(Library *lb, int isbn);
Copy *library_findAvailableCopy(Library *lb, int isbn);
Copy *library_findBorrowedCopy(Library *lb, int isbn);

#endif
