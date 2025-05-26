#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "entities.h"
#include "buffer.h"
#include "utils.h"

void *auxiliar2(void *arg) {
	Library *lb = (Library *)arg;
	char comando;

	while (1) {
		    pthread_mutex_lock(&lb->redibujar_mutex);
		    int necesita_redibujar = lb->redibujar_menu;
		    lb->redibujar_menu = 0;
		    pthread_mutex_unlock(&lb->redibujar_mutex);

		    if (necesita_redibujar) {
		        pthread_mutex_lock(&lb->print_mutex);
		        printf("\n🔄 Solicitud procesada. Menú actualizado:\n");
		        pthread_mutex_unlock(&lb->print_mutex);
		    }

		pthread_mutex_lock(&lb->print_mutex);
		printf("\n ╔══════════════════════════════════════╗\n");
		printf(" ║          MENÚ DE COMANDOS            ║\n");
		printf(" ╠══════════════════════════════════════╣\n");
		printf(" ║  r = Imprimir reporte                ║\n");
		printf(" ║  s = Salir                           ║\n");
		printf(" ╚══════════════════════════════════════╝\n");
		    pthread_mutex_unlock(&lb->print_mutex);

		printf("\nIngrese comando: ");
		scanf(" %c", &comando);
		if (comando == 's') {
			pthread_mutex_lock(&lb->ejecutando_mutex);
			*(lb->ejecutando) = 0;
			saveLibraryToFile(lb, lb->fileOutput);
			pthread_mutex_unlock(&lb->ejecutando_mutex);
			return NULL;
		} else if (comando == 'r') {
			pthread_mutex_lock(&lb->print_mutex);
			library_printReports(lb);
			pthread_mutex_unlock(&lb->print_mutex);
		} else {
			pthread_mutex_lock(&lb->print_mutex);
			printf("\nComando inválido.\n");
			pthread_mutex_unlock(&lb->print_mutex);
		}
	}

	return NULL;
}
