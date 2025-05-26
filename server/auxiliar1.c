#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "buffer.h"
#include "entities.h"

void *auxiliar1(void *arg) {
	Library *lb = (Library *)arg;
	Request req;

	while (1) {
		pthread_mutex_lock(&lb->ejecutando_mutex);
		int en_ejecucion = *(lb->ejecutando);
		pthread_mutex_unlock(&lb->ejecutando_mutex);
		if (!en_ejecucion) break;

		buffer_consume(lb->buf, &req);

		pthread_mutex_lock(&lb->ejecutando_mutex);
		en_ejecucion = *(lb->ejecutando);
		pthread_mutex_unlock(&lb->ejecutando_mutex);
		if (!en_ejecucion) break;

		Book *book = library_findBookByISBN(lb, req.isbn);
		Copy *copy = library_findBorrowedCopy(lb, req.isbn);

		if (copy) {
			time_t t = time(NULL);
			struct tm *now = localtime(&t);

			if (req.operation == 'D') {
				copy->status = 'D';
				snprintf(copy->date, sizeof(copy->date), "%02d-%02d-%04d",
					now->tm_mday, now->tm_mon + 1, now->tm_year + 1900);
				library_addReport(lb, 'D', book->title, book->isbn, copy->id, copy->date);

			} else if (req.operation == 'R') {
				t += 7 * 24 * 60 * 60; // +7 días
				struct tm *new_date = localtime(&t);
				snprintf(copy->date, sizeof(copy->date), "%02d-%02d-%04d",
					new_date->tm_mday, new_date->tm_mon + 1, new_date->tm_year + 1900);;
				library_addReport(lb, 'P', book->title, book->isbn, copy->id, copy->date);
			}
                } else {
			printf("\nDevolución para ISBN %d rechazada. No se encontró copia prestada.\n", req.isbn);
		}
	}
	return NULL;
}
