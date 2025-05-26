#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "entities.h"

void *auxiliar2(void *arg) {
	Library *lb = (Library *)arg;
	char comando;

	while (*(lb->ejecutando)) {
		printf("Ingrese comando (r = reporte, s = salir): ");
		scanf(" %c", &comando);
		if (comando == 's') {
			*(lb->ejecutando) = 0;
		break;
		} else if (comando == 'r') {
            		// mostrarReporte();
		} else {
            		printf("Comando inválido.\n");
		}
	}
	return NULL;
}
