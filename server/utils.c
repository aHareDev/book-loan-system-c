#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"

int readRequest(int fd_read, Request *req) {
	ssize_t bytes = read(fd_read, req, sizeof(Request));
	if (bytes <= 0) {
		fprintf(stderr, "Error o lectura incompleta del pipe receptor\n");
		return 0;
	}
	return 1;
}

int sendResponse(Request *req) {
	int fd_res = open(req->pipeResponse, O_WRONLY);
	if (fd_res < 0) {
		perror("Error al abrir el pipe de respuesta");
		return 0;
	}

	Response res = { .code = 200 };
	snprintf(res.message, sizeof(res.message), "✅ Confirmación: libro recibido '%s' (ISBN: %d)", req->title, req->isbn);

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
	int iBook = 0, jCopy = 0;
	Book book;

	while(fgets(line, sizeof(line), fp)) {
		if (line[0] == '\n' || line[0] == '\r') continue;

		if (sscanf(line, " %99[^,], %d, %d", book.title, &book.isbn, &book.totalCopies) == 3) {
			jCopy = 0;
		} else if (jCopy < book.totalCopies && sscanf(line, " %d, %c, %19[^\n]", 
						&book.copies[jCopy].id, &book.copies[jCopy].status, book.copies[jCopy].date) == 3) {
			book.copies[jCopy].isbn = book.isbn;
			jCopy++;
		}
	}

	if (jCopy == book.totalCopies) {
		if (iBook < MAX_BOOKS) {
			db->books[iBook++] = book;
		} else {
			fprintf(stderr, "Máximo de libros alcanzado.\n");
			return 1;
		}
	}

	fclose(fp);
	return 1;
}

void mostrarSolicitud(Request *req) {
    printf("\n  Solicitud recibida:\n");
    printf("  PID cliente  : %d\n", req->pid);
    printf("  Operación    : %c\n", req->operation);
    printf("  Título libro : %s\n", req->title);
    printf("  ISBN         : %d\n", req->isbn);
    printf("  Pipe respuesta: %s\n\n", req->pipeResponse);
}
