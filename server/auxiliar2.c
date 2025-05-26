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

		fflush(stdout);

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
			scanf(" %c", &comando);
			if (comando == 's') {
				pthread_mutex_lock(&lb->ejecutando_mutex);
				*(lb->ejecutando) = 0;
				saveLibraryToFile(lb, lb->fileOutput);
				pthread_mutex_unlock(&lb->ejecutando_mutex);
				return NULL;
			} else if (comando == 'r') {
				library_printReports(lb);
				lb->interaction = 1;
			} else {
				printf("\nComando inválido.\n");
			}
		}
	}

	return NULL;
}
