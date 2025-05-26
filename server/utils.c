#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"

int readRequest(int fd_read, Request *req) {
	ssize_t bytes = read(fd_read, req, sizeof(Request));
	if (bytes <= 0) {
		return 0;
	}
	return 1;
}

int sendResponse(Request *req, char *response) {
	int fd_res = open(req->pipeResponse, O_WRONLY);
	if (fd_res < 0) {
		perror("Error abriendo el pipe de respuesta");
		return 0;
	}

	Response res = { .code = 200 };
	snprintf(res.message, sizeof(res.message), "%s", response);

	ssize_t bytes = write(fd_res, &res, sizeof(Response));
	if (bytes != sizeof(Response)) {
		perror("Error al escribir la respuesta en el pipe");
		close(fd_res);
		return 0;
	}
	close(fd_res);
	return 1;
}

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

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '\n' || line[0] == '\r') continue;

        char title[MAX_CHARACTERS];
        int isbn, total;

        if (sscanf(line, " %99[^,], %d, %d", title, &isbn, &total) == 3) {
            // Guardar libro anterior si estaba leyendo
            if (leyendoLibro && iBook < MAX_BOOKS) {
                db->books[iBook++] = book;
            }

            // Nuevo libro
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
            if (sscanf(line, " %d, %c, %19[^\n]", &id, &status, date) == 3) {
                book.copies[jCopy].id = id;
                book.copies[jCopy].status = status;
                strcpy(book.copies[jCopy].date, date);
                book.copies[jCopy].isbn = book.isbn;
                jCopy++;
            }
        }
    }

    // Guardar último libro
    if (leyendoLibro && iBook < MAX_BOOKS) {
        db->books[iBook++] = book;
    }

    fclose(fp);
    return 1;
}

int saveLibraryToFile(Library *lib, const char *filename) {
    if (!lib || !filename) return 0;

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Error abriendo archivo de salida");
        return 0;
    }

    for (int i = 0; i < MAX_BOOKS; i++) {
        Book *book = &lib->books[i];
        if (book->isbn == 0 || book->totalCopies == 0) continue;

        fprintf(fp, "%s, %d, %d\n", book->title, book->isbn, book->totalCopies);

        for (int j = 0; j < book->totalCopies; j++) {
            Copy *copy = &book->copies[j];
            fprintf(fp, "%d, %c, %s\n", copy->id, copy->status, copy->date);
        }
    }

    fclose(fp);
    return 1;
}

void library_printReports(Library *library) {
    printf("\n\n ====  REPORTE FINAL ==== \n");
    printf("Status, Nombre del Libro, ISBN, ejemplar, fecha\n");

    for (int i = 0; i < NAME_SIZE && library->reports[i].isbn != 0; i++) {
        Report *r = &library->reports[i];
        printf("%c, %s, %d, %d, %s\n", r->status, r->bookName, r->isbn, r->idCopy, r->date);
    }
}

void showRequest(Request *req) {
    printf("\n\n ==== Solicitud recibida  ==== \n");
    printf("PID cliente  : %d\n", req->pid);
    printf("Operación    : %c\n", req->operation);
    printf("Título libro : %s\n", req->title);
    printf("ISBN         : %d\n\n", req->isbn);
}
