#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"

/**
 * Lee una solicitud (Request) desde el descriptor de archivo dado.
 * @param fd_read Descriptor de archivo desde donde se lee la solicitud.
 * @param req Puntero a la estructura Request donde se almacenará la solicitud leída.
 * @return 1 si la lectura fue exitosa, 0 si hubo error o EOF.
 */d
int readRequest(int fd_read, Request *req) {
	ssize_t bytes = read(fd_read, req, sizeof(Request));
	if (bytes <= 0) {
		return 0;
	}
	return 1;
}

/**
 * Envía una respuesta a un cliente utilizando el pipe indicado en la solicitud.
 * @param req Puntero a la solicitud que contiene el nombre del pipe de respuesta.
 * @param response Mensaje que se enviará como respuesta.
 * @return 1 si la respuesta fue enviada correctamente, 0 en caso de error.
 */
int sendResponse(Request *req, char *response) {
    	// Abrimos el pipe para escritura
	int fd_res = open(req->pipeResponse, O_WRONLY);
	if (fd_res < 0) {
		perror("Error abriendo el pipe de respuesta");
		return 0;
	}

        // Construimos la estructura Response con código 200 (OK) y el mensaje
	Response res = { .code = 200 };
	snprintf(res.message, sizeof(res.message), "%s", response);

	// Escribimos la respuesta en el pipe
	ssize_t bytes = write(fd_res, &res, sizeof(Response));
	if (bytes != sizeof(Response)) {
		perror("Error al escribir la respuesta en el pipe");
		close(fd_res);
		return 0;
	}
	close(fd_res);
	return 1;
}

/**
 * Carga la información de la biblioteca desde un archivo.
 * El archivo debe tener un formato específico:
 * Línea con título, ISBN y total de copias: "Título, ISBN, total"
 * Líneas siguientes con copias: "id, status, fecha"
 * @param db Puntero a la estructura Library donde se almacenarán los datos.
 * @param filename Nombre del archivo de entrada.
 * @return 1 si se cargó exitosamente, 0 en caso de error.
 */
int loadLibraryFromFile(Library *db, char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Error abriendo archivo");
        return 0;
    }

    char line[MAX_CHARACTERS];
    int iBook = 0;
    Book book;
    int jCopy = 0;
    int leyendoLibro = 0;

    // Leer línea por línea
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '\n' || line[0] == '\r') continue;

        char title[MAX_CHARACTERS];
        int isbn, total;

        // Intentamos leer línea de libro con título, isbn y total copias
        if (sscanf(line, " %99[^,], %d, %d", title, &isbn, &total) == 3) {
            // Si ya estábamos leyendo un libro, guardamos el libro anterior
            if (leyendoLibro && iBook < MAX_BOOKS) {
                db->books[iBook++] = book;
            }

            // Comenzamos nuevo libro con datos recién leídos
            strcpy(book.title, title);
            book.isbn = isbn;
            book.totalCopies = total;
            jCopy = 0;
            leyendoLibro = 1;
        }
        else if (leyendoLibro && jCopy < ITEMS_BUFFER) {
            int id;
            char status;
            char date[20];

            // Formato esperado para copia: id, estado, fecha
            if (sscanf(line, " %d, %c, %19[^\n]", &id, &status, date) == 3) {
                book.copies[jCopy].id = id;
                book.copies[jCopy].status = status;
                strcpy(book.copies[jCopy].date, date);
                book.copies[jCopy].isbn = book.isbn;
                jCopy++;
            }
        }
    }

    // Guardar último libro leído si existe espacio
    if (leyendoLibro && iBook < MAX_BOOKS) {
        db->books[iBook++] = book;
    }

    fclose(fp);
    return 1;
}


/**
 * Guarda el estado actual de la biblioteca en un archivo de texto.
 * El formato del archivo es compatible con el método loadLibraryFromFile.
 * @param lib Puntero a la biblioteca a guardar.
 * @param filename Nombre del archivo de salida.
 * @return 1 si se guardó correctamente, 0 en caso de error.
 */
int saveLibraryToFile(Library *lib, const char *filename) {
    if (!lib || !filename) return 0;

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Error abriendo archivo de salida");
        return 0;
    }

    // Recorremos todos los libros
    for (int i = 0; i < MAX_BOOKS; i++) {
        Book *book = &lib->books[i];
        // Ignorar entradas vacías (isbn=0 o sin copias)
        if (book->isbn == 0 || book->totalCopies == 0) continue;

        // Escribimos la línea del libro: título, ISBN, total copias
        fprintf(fp, "%s, %d, %d\n", book->title, book->isbn, book->totalCopies);

        // Escribimos las copias del libro con formato id, estado, fecha
        for (int j = 0; j < book->totalCopies; j++) {
            Copy *copy = &book->copies[j];
            fprintf(fp, "%d, %c, %s\n", copy->id, copy->status, copy->date);
        }
    }

    fclose(fp);
    return 1;
}

/**
 * Imprime en pantalla el reporte de movimientos de la biblioteca.
 * Muestra estado, nombre del libro, ISBN, número de ejemplar y fecha.
 * @param library Puntero a la biblioteca que contiene los reportes.
 */
void library_printReports(Library *library) {
    printf("\n\nREPORTE\n");
    printf("Status, Nombre del Libro, ISBN, ejemplar, fecha\n");

    // Recorremos el arreglo de reportes hasta encontrar uno sin ISBN
    for (int i = 0; i < NAME_SIZE && library->reports[i].isbn != 0; i++) {
        Report *r = &library->reports[i];
        printf("%c, %s, %d, %d, %s\n", r->status, r->bookName, r->isbn, r->idCopy, r->date);
    }
}

/**
 * Muestra por pantalla los detalles de una solicitud recibida.
 * Útil para depuración o logs.
 * @param req Puntero a la solicitud a mostrar.
 */
void showRequest(Request *req) {
    printf("\nSolicitud recibida\n");
    printf("PID cliente  : %d\n", req->pid);
    printf("Operación    : %c\n", req->operation);
    printf("Título libro : %s\n", req->title);
    printf("ISBN         : %d\n", req->isbn);
}
