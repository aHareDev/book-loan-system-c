#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "buffer.h"
#include "entities.h"

int findBookCopy(Library *lb, int isbn, Copy **copy_out);

void *auxiliar1(void *arg) {
	Library *lb = (Library *)arg;
	Request req;

	while (*(lb->ejecutando)) {
		buffer_consume(lb->buf, &req);

		Copy *copy = NULL;
		if (findBookCopy(lb, req.isbn, &copy) == 0 && copy) {
			if (req.operation != 'D') {
				copy->status = 'D';
				time_t t = time(NULL);
				struct tm *now = localtime(&t);
				snprintf(copy->date, sizeof(copy->date), "%02d-%02d-%04d",
					now->tm_mday, now->tm_mon + 1, now->tm_year + 1900);
			}
		} else if (req.operation == 'R') {
			time_t t = time(NULL);
			t += 7 * 24 * 60 * 60; // +7 días
			struct tm *new_date = localtime(&t);
			snprintf(copy->date, sizeof(copy->date), "%02d-%02d-%04d",
				new_date->tm_mday, new_date->tm_mon + 1, new_date->tm_year + 1900);
            }


	}
	return NULL;
}


int findBookCopy(Library *lb, int isbn, Copy **copy_out) {
	if (!lb || !copy_out) return -1;

    for (int i = 0; i < MAX_BOOKS; i++) {
        if (lb->books[i].isbn == isbn) {
            Book *book_out = &lb->books[i];

            // Buscar una copia prestada ('P')
            for (int j = 0; j < lb->books[i].totalCopies; j++) {
                if (lb->books[i].copies[j].status == 'P') {
                    *copy_out = &lb->books[i].copies[j];
                    return 0; // Éxito
                }
            }

            *copy_out = NULL; // No hay copia prestada
            return 0; // Libro encontrado aunque no haya copia prestada
        }
    }
    *copy_out = NULL;
    return -1; // Libro no encontrado

}
