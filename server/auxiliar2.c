/**
 * Función auxiliar2
 * -----------------
 * Hilo que muestra un menú interactivo en consola cuando hay actividad del usuario.
 * Monitorea la entrada estándar (stdin) usando `select()` para no bloquear el hilo principal.
 * Permite al usuario imprimir el historial de operaciones o finalizar el servidor.
 *
 * Parámetros:
 *   arg - puntero a la estructura Library con la base de datos y variables compartidas.
 *
 * Retorna:
 *   NULL al finalizar el hilo.
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/select.h>
#include "entities.h"
#include "buffer.h"
#include "utils.h"

void *auxiliar2(void *arg) {
	Library *lb = (Library *)arg;
	char comando;

	while (1) {
                // Mostrar menú si se habilitó interacción
		pthread_mutex_lock(&lb->interaction_mutex);
		if (lb->interaction) {
			lb->interaction = 0;

			printf("\n ╔══════════════════════════════════════╗\n");
			printf(" ║          MENÚ DE COMANDOS            ║\n");
			printf(" ╠══════════════════════════════════════╣\n");
			printf(" ║  r = Imprimir reporte                ║\n");
			printf(" ║  s = Salir                           ║\n");
			printf(" ╚══════════════════════════════════════╝\n");
			printf("\nIngrese comando: ");
		}
		pthread_mutex_unlock(&lb->interaction_mutex);
		// Asegura que el prompt se muestre inmediatamente
		fflush(stdout);
                // Configurar lectura no bloqueante con timeout de 1 segundo
		fd_set set;
		struct timeval timeout;
		FD_ZERO(&set);
		FD_SET(STDIN_FILENO, &set);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		int rv = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);
		if (rv == -1) {
			perror("select");
			continue;
		} else if (rv == 0) {
			continue; // timeout, vuelve a revisar interaction
		} else {
                        // Leer comando del usuario
			scanf(" %c", &comando);
			if (comando == 's') {
                                // Guardar base de datos y finalizar servidor
				saveLibraryToFile(lb, lb->fileOutput);
				pthread_mutex_lock(&lb->ejecutando_mutex);
				*(lb->ejecutando) = 0;
				pthread_mutex_unlock(&lb->ejecutando_mutex);
				return NULL;
			} else if (comando == 'r') {
                                // Imprimir reporte de operaciones
				library_printReports(lb);
				lb->interaction = 1;
			} else {
				printf("\nComando inválido.\n");
			}
		}
	}

	return NULL;
}
