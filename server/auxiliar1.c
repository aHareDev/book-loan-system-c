/**
 * Función auxiliar1
 * -----------------
 * Hilo consumidor que procesa solicitudes del buffer compartido. Atenderá devoluciones ('D')
 * y renovaciones ('R') de libros, actualizando el estado de las copias correspondientes 
 * y registrando los cambios en el historial de la biblioteca.
 *
 * Parámetros:
 *   arg - puntero a la estructura Library que contiene la base de datos y recursos compartidos.
 *
 * Retorna:
 *   NULL al finalizar el hilo.
 */
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
                // Verificar si el sistema sigue en ejecución
		pthread_mutex_lock(&lb->ejecutando_mutex);
		int en_ejecucion = *(lb->ejecutando);
		pthread_mutex_unlock(&lb->ejecutando_mutex);
		if (!en_ejecucion) break;

                // Consumir una solicitud del buffer
		buffer_consume(lb->buf, &req);

                // Verificar nuevamente si el sistema sigue en ejecución
		pthread_mutex_lock(&lb->ejecutando_mutex);
		en_ejecucion = *(lb->ejecutando);
		pthread_mutex_unlock(&lb->ejecutando_mutex);
		if (!en_ejecucion) break;

                // Buscar libro y copia prestada
		Book *book = library_findBookByISBN(lb, req.isbn);
		Copy *copy = library_findBorrowedCopy(lb, req.isbn);

		if (copy) {
                        // Obtener fecha actual
			time_t t = time(NULL);
			struct tm *now = localtime(&t);

			if (req.operation == 'D') {
                                // Procesar devolución: marcar como disponible y registrar fecha
				copy->status = 'D';
				snprintf(copy->date, sizeof(copy->date), "%02d-%02d-%04d",
					now->tm_mday, now->tm_mon + 1, now->tm_year + 1900);
				library_addReport(lb, 'D', book->title, book->isbn, copy->id, copy->date);

			} else if (req.operation == 'R') {
                                // Procesar renovación: extender fecha en 7 días y registrar
				t += 7 * 24 * 60 * 60; // +7 días
				struct tm *new_date = localtime(&t);
				snprintf(copy->date, sizeof(copy->date), "%02d-%02d-%04d",
					new_date->tm_mday, new_date->tm_mon + 1, new_date->tm_year + 1900);;
				library_addReport(lb, 'P', book->title, book->isbn, copy->id, copy->date);
			}
                } else {
                        // No se encontró copia prestada asociada al ISBN
			printf("\nDevolución para ISBN %d rechazada. No se encontró copia prestada.\n", req.isbn);
		}
	}
	return NULL;
}
