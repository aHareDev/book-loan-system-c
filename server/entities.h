#ifndef COMMON_H
#define COMMON_H

#include <semaphore.h>

#define PIPE_NAME_SIZE 50
#define NAME_SIZE 100
#define MAX_CHARACTERS 256
#define ITEMS_BUFFER 10
#define MAX_BOOKS 30

// Representa una solicitud enviada por un cliente.
typedef struct {
    	char operation;
    	char title[NAME_SIZE];
    	int isbn;
    	pid_t pid; // para identificar qué proceso solicitante envió la solicitud
	char pipeResponse[PIPE_NAME_SIZE];
} Request;

// Estructura para la respuesta enviada a un cliente.
typedef struct {
	int code;
	char message[MAX_CHARACTERS];
} Response;

// Buffer circular sincronizado para almacenar solicitudes.
typedef struct {
	Request items[ITEMS_BUFFER];
	int in;
	int out;
	pthread_mutex_t mutex;
	sem_t full;
	sem_t empty;
} Buffer;

// Registro de operaciones realizadas en la biblioteca.
typedef struct {
	char status;
	char bookName[MAX_CHARACTERS];
	int isbn;
	int idCopy;
	char date[30];
} Report;

// Representa una copia de un libro con estado y fecha.
typedef struct {
	int id;
	int isbn;
	char status; // 'D' o 'P'
	char date[30];
} Copy;

// Estructura que representa un libro con sus copias.
typedef struct {
	char title[MAX_CHARACTERS];
	int isbn;
	int totalCopies;
	Copy copies[ITEMS_BUFFER];
} Book;

// Estructura principal que representa la biblioteca.
typedef struct {
	char fileOutput[MAX_CHARACTERS];
	int interaction;
        Buffer *buf;
        Book books[MAX_BOOKS];
	Report reports[NAME_SIZE];
        int *ejecutando;
	pthread_mutex_t ejecutando_mutex;
	pthread_mutex_t print_mutex;
	pthread_mutex_t interaction_mutex;
} Library;

// Función para agregar un reporte en la biblioteca.
void library_addReport(Library *library, char status, const char *title, int isbn, int idCopy, const char *date);

// Busca un libro en la biblioteca por su ISBN.
Book *library_findBookByISBN(Library *lb, int isbn);

// Busca una copia disponible para préstamo por ISBN.
Copy *library_findAvailableCopy(Library *lb, int isbn);

// Busca una copia actualmente prestada por ISBN.
Copy *library_findBorrowedCopy(Library *lb, int isbn);

#endif
